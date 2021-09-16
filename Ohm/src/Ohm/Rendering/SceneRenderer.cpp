#include "ohmpch.h"
#include "Ohm/Rendering/SceneRenderer.h"
#include "Ohm/Rendering/Renderer.h"
#include "Ohm/Rendering/FrameBuffer.h"
#include "Ohm/Core/Application.h"
#include "Ohm/Rendering/Shader.h"
#include "Ohm/Rendering/UniformBuffer.h"
#include "Ohm/Rendering/RenderCommand.h"

#include <glm/glm.hpp>
#include <glad/glad.h>

namespace Ohm
{
	Ref<Scene> SceneRenderer::s_ActiveScene = nullptr;

	EditorCamera SceneRenderer::s_EditorCamera(45.0f, 1.77776f, 0.1f, 1000.0f);

	Ref<RenderPass> SceneRenderer::s_GeometryPass = nullptr;
	Ref<RenderPass> SceneRenderer::s_PreShadowPass = nullptr;
	Ref<Shader> SceneRenderer::s_DebugDepthShader = nullptr;
	Ref<UniformBuffer> SceneRenderer::s_ShadowUniformbuffer = nullptr;

	void SceneRenderer::LoadScene(const Ref<Scene>& runtimeScene)
	{
		s_EditorCamera.SetPosition(glm::vec3(0.0f, 8.5f, 20.0f));
		s_EditorCamera.SetRotation(glm::vec2(15.0f, 0.0f));
		s_ActiveScene = runtimeScene;
	}

	void SceneRenderer::UnloadScene()
	{
		s_ActiveScene = nullptr;
	}

	void SceneRenderer::InitializePreShadowPass()
	{
		FramebufferSpecification preShadowFrameBufferSpec;

		const uint32_t SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;

		preShadowFrameBufferSpec.AttachmentData = { FramebufferTextureFormat::Depth };
		preShadowFrameBufferSpec.Width = SHADOW_WIDTH;
		preShadowFrameBufferSpec.Height = SHADOW_HEIGHT;

		RenderPassSpecification preShadowRenderPassSpec;
		preShadowRenderPassSpec.ColorWrite = false;
		preShadowRenderPassSpec.TargetFramebuffer = CreateRef<Framebuffer>(preShadowFrameBufferSpec);
		preShadowRenderPassSpec.Shader = ShaderLibrary::Get("PreDepth");
		preShadowRenderPassSpec.Material = CreateRef<Material>("PreDepth", preShadowRenderPassSpec.Shader);
		preShadowRenderPassSpec.Flags |= (uint32_t)RenderFlag::DepthTest;
		preShadowRenderPassSpec.Flags |= (uint32_t)RenderFlag::Blend;
		s_DebugDepthShader = ShaderLibrary::Get("DepthMap");

		s_ShadowUniformbuffer = CreateRef<UniformBuffer>(sizeof(glm::mat4), 2);
		s_PreShadowPass = CreateRef<RenderPass>(preShadowRenderPassSpec);
	}

	void SceneRenderer::InitializeGeometryPass()
	{
		FramebufferSpecification geoFramebufferSpec;

		auto& window = Application::GetApplication().GetWindow();
		geoFramebufferSpec.AttachmentData = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
		geoFramebufferSpec.Width = window.GetWidth();
		geoFramebufferSpec.Height = window.GetHeight();

		RenderPassSpecification geoRenderPassSpec;
		geoRenderPassSpec.ClearColor = glm::vec4(0.3f, 0.3f, 0.3f, 0.0f);
		geoRenderPassSpec.TargetFramebuffer = CreateRef<Framebuffer>(geoFramebufferSpec);
		geoRenderPassSpec.Flags |= (uint32_t)RenderFlag::DepthTest;
		geoRenderPassSpec.Flags |= (uint32_t)RenderFlag::Blend;


		s_GeometryPass = CreateRef<RenderPass>(geoRenderPassSpec);
	}

	void SceneRenderer::PreShadowPass()
	{
		Renderer::BeginPass(s_PreShadowPass);
		auto& light = s_ActiveScene->GetSunLight();

		static glm::mat4 scaleBiasMatrix = glm::scale(glm::mat4(1.0f), { 0.5f, 0.5f, 0.5f }) * glm::translate(glm::mat4(1.0f), { 1, 1, 1 });

		auto& lightTransform = light.GetComponent<TransformComponent>();
		glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, 1.0f, 300.0f);
		glm::mat4 lightView = glm::lookAt(lightTransform.Translation, lightTransform.Forward(), lightTransform.Up());

		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		s_ShadowUniformbuffer->SetData((void*)&lightSpaceMatrix, sizeof(lightSpaceMatrix));

		auto group = s_ActiveScene->m_Registry.group<TransformComponent, MeshRendererComponent>();

		for (auto entity : group)
		{
			auto [transform, meshRenderer] = group.get<TransformComponent, MeshRendererComponent>(entity);

			if (!meshRenderer.MaterialInstance->CastsShadows()) continue;

			s_PreShadowPass->GetRenderPassSpecification().Material->Set<glm::mat4>("u_ModelMatrix", transform.Transform());
			Renderer::DrawMesh(s_EditorCamera, meshRenderer.MeshData, s_PreShadowPass->GetRenderPassSpecification().Material, transform);
		}

		s_PreShadowPass->GetRenderPassSpecification().TargetFramebuffer->BindDepthTexture(2);
		Renderer::EndPass(s_PreShadowPass);
	}

	void SceneRenderer::GeometryPass()
	{
		Renderer::BeginPass(s_GeometryPass);
		auto group = s_ActiveScene->m_Registry.group<TransformComponent, MeshRendererComponent>();

		for (auto entity : group)
		{
			auto [transform, meshRenderer] = group.get<TransformComponent, MeshRendererComponent>(entity);
			s_ActiveScene->SetSceneLightingData(s_EditorCamera);

			// TODO:: Figure out how to not cast shadows but receive them properly.  Currently, if a material is flagged to not
			// cast shadows, their depth is not written to the depth buffer.  This means, they won't be tested against when it comes time
			// to render geometry with lighting.  If an object has a lower depth than the object that isn't to cast shadows but is to receive them,
			// they will receive shadows but any fragment with a greater depth will also receive the shadow.  The expected behavior is that the object
			// should "consume" the shadow.
			meshRenderer.MaterialInstance->CheckShouldReceiveShadows();
			Renderer::DrawMesh(s_EditorCamera, meshRenderer.MeshData, meshRenderer.MaterialInstance, transform);
		}

		Renderer::EndPass(s_GeometryPass);
	}

	void SceneRenderer::InitializePipeline()
	{
		InitializePreShadowPass();
		InitializeGeometryPass();
	}

	void SceneRenderer::SubmitPipeline()
	{
		Renderer::BeginScene();
		PreShadowPass();
		GeometryPass();
		Renderer::EndScene();
	}

	void SceneRenderer::UpdateEditorCamera(float deltaTime)
	{
		s_EditorCamera.Update(deltaTime);
	}

	void SceneRenderer::OnEvent(Event& e)
	{
		s_EditorCamera.OnEvent(e);
	}

	Ref<Framebuffer> SceneRenderer::GetMainColorBuffer()
	{
		if (s_GeometryPass == nullptr || s_GeometryPass->GetRenderPassSpecification().TargetFramebuffer == nullptr)
		{
			OHM_CORE_ERROR("Main Color Buffer not found.  Geometry Pass (main color attachment) or Geometry Pass FBO is nullptr.");
			return nullptr;
		}

		return s_GeometryPass->GetRenderPassSpecification().TargetFramebuffer;
	}

	Ref<Framebuffer> SceneRenderer::GetDepthBuffer()
	{
		if (s_PreShadowPass == nullptr || s_PreShadowPass->GetRenderPassSpecification().TargetFramebuffer == nullptr)
		{
			OHM_CORE_ERROR("Depth Buffer not found.  Pre Depth Pass or Pre Depth Pass or FBO is nullptr.");
			return nullptr;
		}

		return s_PreShadowPass->GetRenderPassSpecification().TargetFramebuffer;
	}

	void SceneRenderer::ValidateResize(glm::vec2 viewportSize)
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