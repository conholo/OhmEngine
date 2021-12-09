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
#include <imgui.h>

namespace Ohm
{
	Ref<Scene> SceneRenderer::s_ActiveScene = nullptr;

	EditorCamera SceneRenderer::s_EditorCamera(45.0f, 1.77776f, 0.1f, 1000.0f);

	Ref<RenderPass> SceneRenderer::s_GeometryPass = nullptr;
	Ref<RenderPass> SceneRenderer::s_ShadowPass = nullptr;
	Ref<RenderPass> SceneRenderer::s_SceneCompositePass = nullptr;

	Ref<Shader> SceneRenderer::s_DebugDepthShader = nullptr;
	Ref<UniformBuffer> SceneRenderer::s_ShadowUniformbuffer = nullptr;

	Ref<SceneRenderer::UIPropertyDrawers> SceneRenderer::s_Drawers = nullptr;
	Ref<SceneRenderer::BloomProperties> SceneRenderer::s_BloomProperties = nullptr;


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

	void SceneRenderer::InitializeUI()
	{
		s_Drawers = CreateRef<SceneRenderer::UIPropertyDrawers>();

		s_Drawers->BloomIntensityDrawer = CreateRef<UI::UIFloat>("Bloom Intensity", &s_BloomProperties->BloomIntensity);
		s_Drawers->BloomThresholdDrawer = CreateRef<UI::UIFloat>("Bloom Threshold", &s_BloomProperties->BloomThreshold);
		s_Drawers->BloomKneeDrawer = CreateRef<UI::UIFloat>("Bloom Knee", &s_BloomProperties->BloomKnee);
		s_Drawers->BloomDirtIntensityDrawer = CreateRef<UI::UIFloat>("Bloom Dirt Intensity", &s_BloomProperties->BloomDirtIntensity);
	}

	void SceneRenderer::InitializeShadowPass()
	{
		FramebufferSpecification shadowFrameBufferSpec;

		const uint32_t SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;

		shadowFrameBufferSpec.AttachmentData = { FramebufferTextureFormat::Depth };
		shadowFrameBufferSpec.Width = SHADOW_WIDTH;
		shadowFrameBufferSpec.Height = SHADOW_HEIGHT;

		RenderPassSpecification shadowRenderPassSpec;
		shadowRenderPassSpec.ColorWrite = false;
		shadowRenderPassSpec.TargetFramebuffer = CreateRef<Framebuffer>(shadowFrameBufferSpec);
		shadowRenderPassSpec.Shader = ShaderLibrary::Get("ShadowMap");
		shadowRenderPassSpec.Material = CreateRef<Material>("ShadowMap", shadowRenderPassSpec.Shader);
		shadowRenderPassSpec.Flags |= (uint32_t)RenderFlag::DepthTest;
		shadowRenderPassSpec.Flags |= (uint32_t)RenderFlag::Blend;
		s_DebugDepthShader = ShaderLibrary::Get("DepthMap");

		s_ShadowUniformbuffer = CreateRef<UniformBuffer>(sizeof(glm::mat4), 2);
		s_ShadowPass = CreateRef<RenderPass>(shadowRenderPassSpec);
	}

	void SceneRenderer::InitializeGeometryPass()
	{
		FramebufferSpecification geoFramebufferSpec;

		auto& window = Application::GetApplication().GetWindow();
		geoFramebufferSpec.AttachmentData = { FramebufferTextureFormat::RGBA32F, FramebufferTextureFormat::Depth };
		geoFramebufferSpec.Width = window.GetWidth();
		geoFramebufferSpec.Height = window.GetHeight();

		RenderPassSpecification geoRenderPassSpec;
		geoRenderPassSpec.ClearColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
		geoRenderPassSpec.TargetFramebuffer = CreateRef<Framebuffer>(geoFramebufferSpec);
		geoRenderPassSpec.Flags |= (uint32_t)RenderFlag::DepthTest;
		geoRenderPassSpec.Flags |= (uint32_t)RenderFlag::Blend;

		s_GeometryPass = CreateRef<RenderPass>(geoRenderPassSpec);
	}

	void SceneRenderer::InitializeBloomPass()
	{
		s_BloomProperties = CreateRef<BloomProperties>();
		s_BloomProperties->BloomComputeTextures.resize(3);
		Texture2DSpecification fileTexSpec =
		{
			TextureUtils::WrapMode::Repeat,
			TextureUtils::FilterMode::Linear,
			TextureUtils::FilterMode::Linear,
			TextureUtils::ImageInternalFormat::FromImage,
			TextureUtils::ImageDataLayout::FromImage,
			TextureUtils::ImageDataType::UByte,
		};

		s_BloomProperties->BloomDirtTexture = CreateRef<Texture2D>("assets/textures/dirt-mask.png", fileTexSpec);

		glm::vec2 windowDimensions = { Application::GetApplication().GetWindow().GetWidth(), Application::GetApplication().GetWindow().GetHeight() };
		uint32_t halfWidth = windowDimensions.x / 2;
		uint32_t halfHeight = windowDimensions.y / 2;
		halfWidth += (s_BloomProperties->BloomWorkGroupSize - (halfWidth % s_BloomProperties->BloomWorkGroupSize));
		halfHeight += (s_BloomProperties->BloomWorkGroupSize - (halfHeight % s_BloomProperties->BloomWorkGroupSize));

		Texture2DSpecification bloomSpec =
		{
			TextureUtils::WrapMode::ClampToEdge,
			TextureUtils::FilterMode::LinearMipLinear,
			TextureUtils::FilterMode::Linear,
			TextureUtils::ImageInternalFormat::RGBA32F,
			TextureUtils::ImageDataLayout::RGBA,
			TextureUtils::ImageDataType::Float,
			halfWidth, halfHeight
		};

		s_BloomProperties->BloomShader = ShaderLibrary::Get("Bloom");;
		s_BloomProperties->BloomComputeTextures.resize(3);
		s_BloomProperties->BloomComputeTextures[0] = CreateRef<Texture2D>(bloomSpec);
		s_BloomProperties->BloomComputeTextures[1] = CreateRef<Texture2D>(bloomSpec);
		s_BloomProperties->BloomComputeTextures[2] = CreateRef<Texture2D>(bloomSpec);
	}

	void SceneRenderer::InitializeCompositePass()
	{
		FramebufferSpecification compositeFrameBufferSpec;
		auto& window = Application::GetApplication().GetWindow();
		compositeFrameBufferSpec.AttachmentData = { FramebufferTextureFormat::RGBA32F, FramebufferTextureFormat::Depth };
		compositeFrameBufferSpec.Width = window.GetWidth();
		compositeFrameBufferSpec.Height = window.GetHeight();

		RenderPassSpecification compositeRenderPassSpec;
		compositeRenderPassSpec.ClearColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
		compositeRenderPassSpec.TargetFramebuffer = CreateRef<Framebuffer>(compositeFrameBufferSpec);
		compositeRenderPassSpec.Flags |= (uint32_t)RenderFlag::DepthTest;
		compositeRenderPassSpec.Flags |= (uint32_t)RenderFlag::Blend;
		compositeRenderPassSpec.Shader = ShaderLibrary::Get("SceneComposite");

		s_SceneCompositePass = CreateRef<RenderPass>(compositeRenderPassSpec);
	}

	void SceneRenderer::ShadowPass()
	{
		Renderer::BeginPass(s_ShadowPass);
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

			if (!meshRenderer.IsComplete() || !meshRenderer.MaterialInstance->CastsShadows()) continue;

			s_ShadowPass->GetRenderPassSpecification().Material->Set<glm::mat4>("u_ModelMatrix", transform.Transform());
			Renderer::DrawMesh(s_EditorCamera, meshRenderer.MeshData, s_ShadowPass->GetRenderPassSpecification().Material, transform);
		}

		s_ShadowPass->GetRenderPassSpecification().TargetFramebuffer->BindDepthTexture(2);
		Renderer::EndPass(s_ShadowPass);
	}

	void SceneRenderer::GeometryPass()
	{
		Renderer::BeginPass(s_GeometryPass);
		auto group = s_ActiveScene->m_Registry.group<TransformComponent, MeshRendererComponent>();

		for (auto entity : group)
		{
			auto [transform, meshRenderer] = group.get<TransformComponent, MeshRendererComponent>(entity);

			if (!meshRenderer.IsComplete()) continue;

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

	void SceneRenderer::BloomPass()
	{
		s_BloomProperties->BloomShader->Bind();

		struct BloomConstants
		{
			glm::vec4 Params;
			float LOD = 0.0f;
			int Mode = 0;
		} bloomConstants;

		bloomConstants.Params = { s_BloomProperties->BloomThreshold, s_BloomProperties->BloomThreshold - s_BloomProperties->BloomKnee, s_BloomProperties->BloomKnee * 2.0f, 0.25f / s_BloomProperties->BloomKnee };

		//------------------ MODE_PREFILTER -----------------//
		uint32_t workGroupsX = s_BloomProperties->BloomComputeTextures[0]->GetWidth() / s_BloomProperties->BloomWorkGroupSize;
		uint32_t workGroupsY = s_BloomProperties->BloomComputeTextures[0]->GetHeight() / s_BloomProperties->BloomWorkGroupSize;

		{
			bloomConstants.Mode = 0;
			s_BloomProperties->BloomShader->UploadUniformFloat4("u_Params", bloomConstants.Params);
			s_BloomProperties->BloomShader->UploadUniformFloat("u_LOD", bloomConstants.LOD);
			s_BloomProperties->BloomShader->UploadUniformInt("u_Mode", bloomConstants.Mode);
			s_GeometryPass->GetRenderPassSpecification().TargetFramebuffer->BindColorAttachmentTexture(0, 0);
			s_BloomProperties->BloomShader->UploadUniformInt("u_Texture", 0);
			s_BloomProperties->BloomComputeTextures[0]->BindToImageSlot(0, 0, TextureUtils::TextureAccessLevel::WriteOnly, TextureUtils::TextureShaderDataFormat::RGBA32F);

			s_BloomProperties->BloomShader->DispatchCompute(workGroupsX, workGroupsY, 1);
			s_BloomProperties->BloomShader->EnableShaderImageAccessBarrierBit();

			s_GeometryPass->GetRenderPassSpecification().TargetFramebuffer->UnbindAttachmentTexture(0);
		}
		//------------------ MODE_PREFILTER -----------------//


		//------------------ MODE_DOWNSAMPLE -----------------//
		bloomConstants.Mode = 1;
		uint32_t mips = s_BloomProperties->BloomComputeTextures[0]->GetMipCount() - 2;

		for (uint32_t mip = 1; mip < mips; mip++)
		{
			{
				auto [mipWidth, mipHeight] = s_BloomProperties->BloomComputeTextures[0]->GetMipDimensions(mip);
				workGroupsX = (uint32_t)glm::ceil((float)mipWidth / (float)s_BloomProperties->BloomWorkGroupSize);
				workGroupsY = (uint32_t)glm::ceil((float)mipHeight / (float)s_BloomProperties->BloomWorkGroupSize);

				bloomConstants.LOD = mip - 1.0f;
				// Write to 1
				s_BloomProperties->BloomComputeTextures[1]->BindToImageSlot(0, mip, TextureUtils::TextureAccessLevel::WriteOnly, TextureUtils::TextureShaderDataFormat::RGBA32F);
				// Read from 0 (starts pre-filtered)
				s_BloomProperties->BloomComputeTextures[0]->BindToSamplerSlot(0);
				s_BloomProperties->BloomShader->UploadUniformInt("u_Texture", 0);
				s_BloomProperties->BloomShader->UploadUniformInt("u_Mode", bloomConstants.Mode);
				s_BloomProperties->BloomShader->UploadUniformFloat("u_LOD", bloomConstants.LOD);
				s_BloomProperties->BloomShader->DispatchCompute(workGroupsX, workGroupsY, 1);
				s_BloomProperties->BloomShader->EnableShaderImageAccessBarrierBit();
			}

			{
				bloomConstants.LOD = mip;
				// Write to 0
				s_BloomProperties->BloomComputeTextures[0]->BindToImageSlot(0, mip, TextureUtils::TextureAccessLevel::WriteOnly, TextureUtils::TextureShaderDataFormat::RGBA32F);
				// Read from 1
				s_BloomProperties->BloomComputeTextures[1]->BindToSamplerSlot(0);

				s_BloomProperties->BloomShader->UploadUniformInt("u_Texture", 0);
				s_BloomProperties->BloomShader->UploadUniformInt("u_Mode", bloomConstants.Mode);
				s_BloomProperties->BloomShader->UploadUniformFloat("u_LOD", bloomConstants.LOD);
				s_BloomProperties->BloomShader->DispatchCompute(workGroupsX, workGroupsY, 1);
				s_BloomProperties->BloomShader->EnableShaderImageAccessBarrierBit();
			}
		}
		//------------------ MODE_DOWNSAMPLE -----------------//

		//------------------ MODE_UPSAMPLE_FIRST -----------------//
		{
			bloomConstants.Mode = 2;
			bloomConstants.LOD--;
			// Write to 2 at smallest image in up-sampling mip chain
			s_BloomProperties->BloomComputeTextures[2]->BindToImageSlot(0, mips - 2, TextureUtils::TextureAccessLevel::WriteOnly, TextureUtils::TextureShaderDataFormat::RGBA32F);
			// Read from 0 (fully down-sampled)
			s_BloomProperties->BloomComputeTextures[0]->BindToSamplerSlot(0);

			s_BloomProperties->BloomShader->UploadUniformInt("u_Texture", 0);
			s_BloomProperties->BloomShader->UploadUniformInt("u_Mode", bloomConstants.Mode);
			s_BloomProperties->BloomShader->UploadUniformFloat("u_LOD", bloomConstants.LOD);

			auto [mipWidth, mipHeight] = s_BloomProperties->BloomComputeTextures[2]->GetMipDimensions(mips - 2);
			workGroupsX = (uint32_t)glm::ceil((float)mipWidth / (float)s_BloomProperties->BloomWorkGroupSize);
			workGroupsY = (uint32_t)glm::ceil((float)mipHeight / (float)s_BloomProperties->BloomWorkGroupSize);
			s_BloomProperties->BloomShader->DispatchCompute(workGroupsX, workGroupsY, 1);
			s_BloomProperties->BloomShader->EnableShaderImageAccessBarrierBit();
		}
		//------------------ MODE_UPSAMPLE_FIRST -----------------//


		//------------------ UPSAMPLE -----------------//
		{
			bloomConstants.Mode = 3;
			for (int32_t mip = mips - 3; mip >= 0; mip--)
			{
				auto [mipWidth, mipHeight] = s_BloomProperties->BloomComputeTextures[2]->GetMipDimensions(mip);
				workGroupsX = (uint32_t)glm::ceil((float)mipWidth / (float)s_BloomProperties->BloomWorkGroupSize);
				workGroupsY = (uint32_t)glm::ceil((float)mipHeight / (float)s_BloomProperties->BloomWorkGroupSize);

				bloomConstants.LOD = mip;

				// Write to 2
				s_BloomProperties->BloomShader->EnableShaderImageAccessBarrierBit();
				s_BloomProperties->BloomComputeTextures[2]->BindToImageSlot(0, mip, TextureUtils::TextureAccessLevel::WriteOnly, TextureUtils::TextureShaderDataFormat::RGBA32F);
				// Read from 0	
				s_BloomProperties->BloomComputeTextures[0]->BindToSamplerSlot(0);
				s_BloomProperties->BloomShader->UploadUniformInt("u_Texture", 0);
				s_BloomProperties->BloomComputeTextures[2]->BindToSamplerSlot(1);
				s_BloomProperties->BloomShader->UploadUniformInt("u_BloomTexture", 1);
				s_BloomProperties->BloomShader->UploadUniformInt("u_Mode", bloomConstants.Mode);
				s_BloomProperties->BloomShader->UploadUniformFloat("u_LOD", bloomConstants.LOD);
				s_BloomProperties->BloomShader->DispatchCompute(workGroupsX, workGroupsY, 1);
				s_BloomProperties->BloomShader->EnableShaderImageAccessBarrierBit();
			}
		}
		//------------------ UPSAMPLE -----------------//

		for (auto tex : s_BloomProperties->BloomComputeTextures)
			tex->UnbindFromSamplerSlot();
		s_BloomProperties->BloomShader->Unbind();
	}

	void SceneRenderer::CompositePass()
	{
		Renderer::BeginPass(s_SceneCompositePass);

		s_SceneCompositePass->GetRenderPassSpecification().Shader->Bind();
		s_GeometryPass->GetRenderPassSpecification().TargetFramebuffer->BindColorAttachmentTexture(0, 0);
		s_BloomProperties->BloomComputeTextures[2]->BindToSamplerSlot(1);
		s_BloomProperties->BloomDirtTexture->BindToSamplerSlot(2);
		s_SceneCompositePass->GetRenderPassSpecification().Shader->UploadUniformInt("u_SceneTexture", 0);
		s_SceneCompositePass->GetRenderPassSpecification().Shader->UploadUniformInt("u_BloomTexture", 1);
		s_SceneCompositePass->GetRenderPassSpecification().Shader->UploadUniformInt("u_BloomDirtTexture", 2);
		s_SceneCompositePass->GetRenderPassSpecification().Shader->UploadUniformFloat("u_Exposure", s_Drawers->Exposure);
		s_SceneCompositePass->GetRenderPassSpecification().Shader->UploadUniformFloat("u_BloomIntensity", s_BloomProperties->BloomIntensity);
		s_SceneCompositePass->GetRenderPassSpecification().Shader->UploadUniformFloat("u_BloomDirtIntensity", s_BloomProperties->BloomDirtIntensity);

		Renderer::DrawFullScreenQuad();

		s_SceneCompositePass->GetRenderPassSpecification().Shader->Unbind();
		s_GeometryPass->GetRenderPassSpecification().TargetFramebuffer->UnbindAttachmentTexture(0);
		s_BloomProperties->BloomComputeTextures[2]->UnbindFromSamplerSlot(1);
		s_BloomProperties->BloomDirtTexture->UnbindFromSamplerSlot(2);

		Renderer::EndPass(s_SceneCompositePass);
	}


	void SceneRenderer::DrawSceneRendererUI(const glm::vec2 viewportSize)
	{
		s_Drawers->ExposureDrawer->Draw();
		s_Drawers->BloomIntensityDrawer->Draw();
		s_Drawers->BloomThresholdDrawer->Draw();
		s_Drawers->BloomKneeDrawer->Draw();
		s_Drawers->BloomDirtIntensityDrawer->Draw();

		glm::vec2 windowSize = { Application::GetApplication().GetWindow().GetWidth(), Application::GetApplication().GetWindow().GetHeight() };
		float aspect = (float)viewportSize.x / (float)viewportSize.y;
		ImGui::Image((ImTextureID)s_BloomProperties->BloomComputeTextures[0]->GetID(), { 300 * aspect, 300 }, { 0, 1 }, { 1, 0 });
		ImGui::Image((ImTextureID)s_BloomProperties->BloomComputeTextures[1]->GetID(), { 300 * aspect, 300 }, { 0, 1 }, { 1, 0 });
		ImGui::Image((ImTextureID)s_BloomProperties->BloomComputeTextures[2]->GetID(), { 300 * aspect, 300 }, { 0, 1 }, { 1, 0 });
	}

	void SceneRenderer::InitializePipeline()
	{
		InitializeShadowPass();
		InitializeGeometryPass();
		InitializeBloomPass();
		InitializeCompositePass();

		InitializeUI();
	}

	void SceneRenderer::SubmitPipeline()
	{
		Renderer::BeginScene();
		ShadowPass();
		GeometryPass();
		BloomPass();
		CompositePass();
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
		if (s_SceneCompositePass == nullptr || s_SceneCompositePass->GetRenderPassSpecification().TargetFramebuffer == nullptr)
		{
			OHM_CORE_ERROR("Main Color Buffer not found.  Geometry Pass (main color attachment) or Geometry Pass FBO is nullptr.");
			return nullptr;
		}

		return s_SceneCompositePass->GetRenderPassSpecification().TargetFramebuffer;
	}

	Ref<Framebuffer> SceneRenderer::GetDepthBuffer()
	{
		if (s_ShadowPass == nullptr || s_ShadowPass->GetRenderPassSpecification().TargetFramebuffer == nullptr)
		{
			OHM_CORE_ERROR("Depth Buffer not found.  Shadow Pass or FBO is nullptr.");
			return nullptr;
		}

		return s_ShadowPass->GetRenderPassSpecification().TargetFramebuffer;
	}

	static bool NeedsResize(const RenderPassSpecification& spec, const glm::vec2& viewportSize)
	{
		FramebufferSpecification specification = spec.TargetFramebuffer->GetFrameBufferSpecification();
		return viewportSize.x > 0.0f && viewportSize.y > 0.0f && (specification.Width != viewportSize.x || specification.Height != viewportSize.y);
	}

	void SceneRenderer::ValidateResize(glm::vec2 viewportSize)
	{
		if (s_GeometryPass == nullptr || s_GeometryPass->GetRenderPassSpecification().TargetFramebuffer == nullptr)
		{
			OHM_CORE_ERROR("Unable to resize framebuffer/viewport.  Geometry Pass (main color attachment) or Geometry Pass FBO is nullptr.");
			return;
		}

		if (s_SceneCompositePass == nullptr || s_SceneCompositePass->GetRenderPassSpecification().TargetFramebuffer == nullptr)
		{
			OHM_CORE_ERROR("Unable to resize framebuffer/viewport.  Composite Pass (main color attachment) or Composite Pass FBO is nullptr.");
			return;
		}

		if (NeedsResize(s_GeometryPass->GetRenderPassSpecification(), viewportSize) || NeedsResize(s_SceneCompositePass->GetRenderPassSpecification(), viewportSize))
		{
			s_GeometryPass->GetRenderPassSpecification().TargetFramebuffer->Resize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
			s_SceneCompositePass->GetRenderPassSpecification().TargetFramebuffer->Resize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);

			uint32_t halfWidth = viewportSize.x / 2;
			uint32_t halfHeight = viewportSize.y / 2;
			halfWidth += (s_BloomProperties->BloomWorkGroupSize - (halfWidth % s_BloomProperties->BloomWorkGroupSize));
			halfHeight += (s_BloomProperties->BloomWorkGroupSize - (halfHeight % s_BloomProperties->BloomWorkGroupSize));
				
			s_BloomProperties->BloomComputeTextures[0]->Resize(halfWidth, halfHeight);
			s_BloomProperties->BloomComputeTextures[1]->Resize(halfWidth, halfHeight);
			s_BloomProperties->BloomComputeTextures[2]->Resize(halfWidth, halfHeight);

			s_EditorCamera.SetViewportSize(viewportSize.x, viewportSize.y);
		}
	}
}