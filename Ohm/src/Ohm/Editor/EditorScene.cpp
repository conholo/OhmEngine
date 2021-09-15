#include "ohmpch.h"
#include "Ohm/Editor/EditorScene.h"
#include "Ohm/Rendering/Renderer.h"
#include "Ohm/Rendering/FrameBuffer.h"
#include "Ohm/Core/Application.h"
#include "Ohm/Rendering/Shader.h"
#include "Ohm/Rendering/UniformBuffer.h"

#include <glm/glm.hpp>
#include <glad/glad.h>

namespace Ohm
{
	Ref<Scene> EditorScene::s_ActiveScene = nullptr;

	EditorCamera EditorScene::s_EditorCamera(45.0f, 1.77776f, 0.1f, 1000.0f);

	Ref<RenderPass> EditorScene::s_GeometryPass = nullptr;
	Ref<RenderPass> EditorScene::s_PreDepthPass = nullptr;
	Ref<Shader> EditorScene::s_DebugDepthShader = nullptr;
	Ref<UniformBuffer> EditorScene::s_ShadowUniformbuffer = nullptr;

	void EditorScene::LoadScene(const Ref<Scene>& runtimeScene)
	{
		s_EditorCamera.SetPosition(glm::vec3(0.0f, 8.5f, 20.0f));
		s_EditorCamera.SetRotation(glm::vec2(15.0f, 0.0f));
		s_ActiveScene = runtimeScene;
	}

	void EditorScene::UnloadScene()
	{
		s_ActiveScene = nullptr;
	}

	void EditorScene::InitializeDepthPass()
	{
		FramebufferSpecification preShadowFrameBufferSpec;

		const uint32_t SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;

		preShadowFrameBufferSpec.AttachmentData = { FramebufferTextureFormat::Depth };
		preShadowFrameBufferSpec.Width = SHADOW_WIDTH;
		preShadowFrameBufferSpec.Height = SHADOW_HEIGHT;

		RenderPassSpecification preShadowRenderPassSpec;
		preShadowRenderPassSpec.ColorWrite = false;
		preShadowRenderPassSpec.TargetFramebuffer = CreateRef<Framebuffer>(preShadowFrameBufferSpec);
		preShadowRenderPassSpec.Shader = CreateRef<Shader>("assets/shaders/PreDepth.shader");
		s_DebugDepthShader = CreateRef<Shader>("assets/shaders/DepthMap.shader");

		s_ShadowUniformbuffer = CreateRef<UniformBuffer>(sizeof(glm::mat4), 2);

		s_PreDepthPass = CreateRef<RenderPass>(preShadowRenderPassSpec);
	}

	void EditorScene::InitializeGeometryPass()
	{
		FramebufferSpecification geoFramebufferSpec;

		auto& window = Application::GetApplication().GetWindow();
		geoFramebufferSpec.AttachmentData = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
		geoFramebufferSpec.Width = window.GetWidth();
		geoFramebufferSpec.Height = window.GetHeight();

		RenderPassSpecification geoRenderPassSpec;
		geoRenderPassSpec.ClearColor = glm::vec4(0.3f, 0.3f, 0.3f, 0.0f);
		geoRenderPassSpec.TargetFramebuffer = CreateRef<Framebuffer>(geoFramebufferSpec);

		s_GeometryPass = CreateRef<RenderPass>(geoRenderPassSpec);
	}

	void EditorScene::DepthPass()
	{
		Renderer::BeginPass(s_PreDepthPass);
		auto& light = s_ActiveScene->GetSunLight();

		auto& lightTransform = light.GetComponent<TransformComponent>();

		glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, 1.0f, 300.0f);

		glm::mat4 lightView = glm::lookAt(lightTransform.Translation, lightTransform.Forward(), lightTransform.Up());

		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		Ref<Shader>& preDepthShader = s_PreDepthPass->GetRenderPassSpecification().Shader;

		s_ShadowUniformbuffer->SetData((void*)&lightSpaceMatrix, sizeof(lightSpaceMatrix));

		preDepthShader->Bind();

		auto group = s_ActiveScene->m_Registry.group<TransformComponent, MeshRendererComponent>();

		for (auto entity : group)
		{
			auto [transform, meshRenderer] = group.get<TransformComponent, MeshRendererComponent>(entity);
			preDepthShader->UploadUniformMat4("u_ModelMatrix", transform.Transform());
			Renderer::DrawGeometry(meshRenderer);
		}

		preDepthShader->Unbind();

		Renderer::EndPass(s_PreDepthPass);

		glBindTextureUnit(2, s_PreDepthPass->GetRenderPassSpecification().TargetFramebuffer->GetDepthAttachmentID());
	}

	void EditorScene::GeometryPass()
	{
		Renderer::BeginPass(s_GeometryPass);
		auto group = s_ActiveScene->m_Registry.group<TransformComponent, MeshRendererComponent>();

		for (auto entity : group)
		{
			auto [transform, meshRenderer] = group.get<TransformComponent, MeshRendererComponent>(entity);
			s_ActiveScene->SetSceneLightingData(s_EditorCamera);
			Renderer::DrawMeshWithMaterial(s_EditorCamera, meshRenderer, transform);
		}

		//s_DebugDepthShader->Bind();
		//glBindTextureUnit(2, s_PreDepthPass->GetRenderPassSpecification().TargetFramebuffer->GetDepthAttachmentID());
		//s_DebugDepthShader->UploadUniformFloat("u_NearPlane", 1.0f);
		//s_DebugDepthShader->UploadUniformFloat("u_FarPlane", 10.0f);
		//s_DebugDepthShader->UploadUniformInt("sampler_DepthMap", 2);
		//Renderer::DrawFullScreenQuad();
		//s_DebugDepthShader->Unbind();


		Renderer::EndPass(s_GeometryPass);
	}

	void EditorScene::InitializePipeline()
	{
		InitializeDepthPass();
		InitializeGeometryPass();
	}

	void EditorScene::ExecutePipeline()
	{
		Renderer::BeginScene();
		DepthPass();
		GeometryPass();
		Renderer::EndScene();
	}

	void EditorScene::UpdateEditorCamera(Time dt)
	{
		s_EditorCamera.Update(dt);
	}

	void EditorScene::OnEvent(Event& e)
	{
		s_EditorCamera.OnEvent(e);
	}

	Ref<Framebuffer> EditorScene::GetMainColorBuffer()
	{
		if (s_GeometryPass == nullptr || s_GeometryPass->GetRenderPassSpecification().TargetFramebuffer == nullptr)
		{
			OHM_CORE_ERROR("Main Color Buffer not found.  Geometry Pass (main color attachment) or Geometry Pass FBO is nullptr.");
			return nullptr;
		}

		return s_GeometryPass->GetRenderPassSpecification().TargetFramebuffer;
	}

	Ref<Framebuffer> EditorScene::GetDepthBuffer()
	{
		if (s_PreDepthPass == nullptr || s_PreDepthPass->GetRenderPassSpecification().TargetFramebuffer == nullptr)
		{
			OHM_CORE_ERROR("Depth Buffer not found.  Pre Depth Pass or Pre Depth Pass or FBO is nullptr.");
			return nullptr;
		}

		return s_PreDepthPass->GetRenderPassSpecification().TargetFramebuffer;
	}

	void EditorScene::ValidateResize(glm::vec2 viewportSize)
	{
		if (s_GeometryPass == nullptr || s_GeometryPass->GetRenderPassSpecification().TargetFramebuffer == nullptr)
		{
			OHM_CORE_ERROR("Unable to resize framebuffer/viewport.  Geometry Pass (main color attachment) or Geometry Pass FBO is nullptr.");
			return;
		}

		if (FramebufferSpecification specification = s_GeometryPass->GetRenderPassSpecification().TargetFramebuffer->GetFrameBufferSpecification();
			viewportSize.x > 0.0f && viewportSize.y > 0.0f &&
			(specification.Width != viewportSize.x || specification.Height != viewportSize.y))
		{
			s_GeometryPass->GetRenderPassSpecification().TargetFramebuffer->Resize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
			s_EditorCamera.SetViewportSize(viewportSize.x, viewportSize.y);
		}
	}

}