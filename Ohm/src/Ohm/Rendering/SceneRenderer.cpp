#include "ohmpch.h"
#include "Ohm/Rendering/SceneRenderer.h"
#include "Ohm/Rendering/Renderer.h"
#include "Ohm/Rendering/FrameBuffer.h"
#include "Ohm/Core/Application.h"
#include "Ohm/Rendering/Shader.h"
#include "Ohm/Rendering/UniformBuffer.h"
#include "Ohm/Rendering/RenderCommand.h"
#include "Ohm/UI/PropertyDrawer.h"

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

	Ref<Mesh> SceneRenderer::s_EnvironmentCube = nullptr;
	Ref<TextureCube> SceneRenderer::s_EnvironmentMap = nullptr;
	Ref<Shader> SceneRenderer::s_EnvironmentMapShader = nullptr;
	Ref<Shader> SceneRenderer::s_SkyboxShader = nullptr;

	Ref<SceneRenderer::SkyboxProperties> SceneRenderer::s_SkyboxProperties = nullptr;
	Ref<SceneRenderer::SceneRenderProperties> SceneRenderer::s_SceneRenderProperties = nullptr;
	Ref<SceneRenderer::BloomProperties> SceneRenderer::s_BloomProperties = nullptr;
	Ref<SceneRenderer::GridSettings> SceneRenderer::s_GridSettings = nullptr;
	Ref<SceneRenderer::GridData> SceneRenderer::s_GridData = nullptr;

	void SceneRenderer::LoadScene(const Ref<Scene>& runtimeScene)
	{
		s_EditorCamera.SetPosition(glm::vec3(0.0f, 8.5f, 20.0f));
		s_EditorCamera.SetRotation(glm::vec2(15.0f, 0.0f));
		s_ActiveScene = runtimeScene;
		s_SceneRenderProperties = CreateRef<SceneRenderer::SceneRenderProperties>();
	}

	void SceneRenderer::UnloadScene()
	{
		s_ActiveScene = nullptr;
	}

	void SceneRenderer::InitializeUI()
	{
	}

	void SceneRenderer::InitializeSkybox()
	{
		TextureCubeSpecification spec =
		{
			"Sky Box",
			TextureUtils::WrapMode::ClampToEdge,
			TextureUtils::FilterMode::Linear,
			TextureUtils::FilterMode::Linear,
			TextureUtils::ImageInternalFormat::RGBA32F,
			TextureUtils::ImageDataLayout::RGBA,
			TextureUtils::ImageDataType::Float,
			1024, 1024
		};

		ShaderLibrary::Load("assets/shaders/Skybox.shader");
		ShaderLibrary::Load("assets/shaders/Preetham.shader");

		s_SkyboxProperties = CreateRef<SceneRenderer::SkyboxProperties>();

		s_SkyboxShader = ShaderLibrary::Get("Skybox");
		s_EnvironmentMapShader = ShaderLibrary::Get("Preetham");

		s_EnvironmentCube = Mesh::CreateUnitCube();
		s_EnvironmentMap = CreateRef<TextureCube>(spec);
	}

	void SceneRenderer::InitializeGrid()
	{
		ShaderLibrary::Load("assets/shaders/EditorGrid.shader");

		s_GridData = CreateRef<GridData>();
		s_GridSettings = CreateRef<GridSettings>();

		s_GridData->FullScreenQuad = Mesh::CreatePrimitive(Primitive::FullScreenQuad);
		s_GridData->GridShader = ShaderLibrary::Get("EditorGrid");
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
			"Bloom Dirt Mask",
			TextureUtils::WrapMode::Repeat,
			TextureUtils::FilterMode::Linear,
			TextureUtils::FilterMode::Linear,
			TextureUtils::ImageInternalFormat::FromImage,
			TextureUtils::ImageDataLayout::FromImage,
			TextureUtils::ImageDataType::UByte,
		};

		s_BloomProperties->BloomDirtTexture = TextureLibrary::Load(fileTexSpec, "assets/textures/dirt-mask.png");

		glm::vec2 windowDimensions = { Application::GetApplication().GetWindow().GetWidth(), Application::GetApplication().GetWindow().GetHeight() };
		uint32_t halfWidth = windowDimensions.x / 2;
		uint32_t halfHeight = windowDimensions.y / 2;
		halfWidth += (s_BloomProperties->BloomWorkGroupSize - (halfWidth % s_BloomProperties->BloomWorkGroupSize));
		halfHeight += (s_BloomProperties->BloomWorkGroupSize - (halfHeight % s_BloomProperties->BloomWorkGroupSize));

		Texture2DSpecification bloomSpec =
		{
			"Bloom 1",
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
		bloomSpec.Name = "Bloom 2";
		s_BloomProperties->BloomComputeTextures[1] = CreateRef<Texture2D>(bloomSpec);
		bloomSpec.Name = "Bloom 3";
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
		if (!s_ActiveScene->HasMainLight()) return;

		auto& light = s_ActiveScene->GetSunLight();
		Renderer::BeginPass(s_ShadowPass);
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

			if (!meshRenderer.IsComplete()) continue;

			s_ShadowPass->GetRenderPassSpecification().Material->Set<glm::mat4>("u_ModelMatrix", transform.Transform());
			Renderer::DrawMesh(s_EditorCamera, meshRenderer.MeshData, s_ShadowPass->GetRenderPassSpecification().Material, transform);
		}

		s_ShadowPass->GetRenderPassSpecification().TargetFramebuffer->BindDepthTexture(0);
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

			if (s_ActiveScene->HasMainLight())
				s_ActiveScene->SetSceneLightingData(s_EditorCamera);

			meshRenderer.MaterialInstance->BindActiveTextures();
			Renderer::DrawMesh(s_EditorCamera, meshRenderer.MeshData, meshRenderer.MaterialInstance, transform);
		}

		DrawSkybox();
		if(s_GridSettings->DrawGrid)
			DrawGrid();

		Renderer::EndPass(s_GeometryPass);
	}

	void SceneRenderer::DrawSkybox()
	{
		s_EnvironmentMapShader->Bind();
		s_EnvironmentMap->BindToImageSlot(0, 0, TextureUtils::TextureAccessLevel::WriteOnly, TextureUtils::TextureShaderDataFormat::RGBA32F);
		s_EnvironmentMapShader->UploadUniformFloat3("u_TAI", glm::vec3(s_SkyboxProperties->Turbidity, s_SkyboxProperties->Azimuth, s_SkyboxProperties->Inclination));
		s_EnvironmentMapShader->DispatchCompute(s_EnvironmentMap->GetWidth() / 32, s_EnvironmentMap->GetHeight() / 32, 6);
		s_EnvironmentMapShader->EnableShaderImageAccessBarrierBit();

		s_SkyboxShader->Bind();
		s_EnvironmentMap->BindToSamplerSlot(0);
		s_SkyboxShader->UploadUniformInt("u_Texture", 0);
		s_SkyboxShader->UploadUniformMat4("u_ViewProjection", glm::inverse(s_EditorCamera.GetViewProjection()));
		s_SkyboxShader->UploadUniformFloat("u_Intensity", s_SkyboxProperties->Intensity);
		s_SkyboxShader->UploadUniformFloat("u_TextureLOD", (float)s_SkyboxProperties->LOD);

		Renderer::DrawUnitCube();
		s_EnvironmentMap->UnbindFromSamplerSlot(0);
	}

	void SceneRenderer::DrawGrid()
	{
		s_GridData->GridShader->Bind();
		s_GridData->GridShader->UploadUniformFloat("u_InnerGridScale", s_GridSettings->InnerScale);
		s_GridData->GridShader->UploadUniformFloat("u_OuterGridScale", s_GridSettings->OuterScale);
		s_GridData->GridShader->UploadUniformFloat3("u_GridColor", s_GridSettings->GridColor);
		s_GridData->GridShader->UploadUniformMat4("u_ViewMatrix", s_EditorCamera.GetView());
		s_GridData->GridShader->UploadUniformMat4("u_ProjectionMatrix", s_EditorCamera.GetProjection());
		s_GridData->GridShader->UploadUniformMat4("u_InverseViewMatrix", glm::inverse(s_EditorCamera.GetView()));
		s_GridData->GridShader->UploadUniformMat4("u_InverseProjectionMatrix", glm::inverse(s_EditorCamera.GetProjection()));

		Renderer::DrawMesh(s_EditorCamera, s_GridData->FullScreenQuad);
		s_GridData->GridShader->Unbind();
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

		//------------------ PREFILTER -----------------//
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
		//------------------ PREFILTER -----------------//


		//------------------ DOWNSAMPLE -----------------//
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
		//------------------ DOWNSAMPLE -----------------//

		//------------------ UPSAMPLE_FIRST -----------------//
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
		//------------------ UPSAMPLE_FIRST -----------------//


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
		s_SceneCompositePass->GetRenderPassSpecification().Shader->UploadUniformFloat("u_Exposure", s_SceneRenderProperties->Exposure);
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
		if (ImGui::CollapsingHeader("Scene Render Settings"))
		{
			UI::UIFloat::Draw("Exposure", &s_SceneRenderProperties->Exposure);
		}

		if (ImGui::CollapsingHeader("Skybox Settings"))
		{
			UI::UIFloat::Draw("Turbidity", &s_SkyboxProperties->Turbidity);
			UI::UIFloat::Draw("Azimuth", &s_SkyboxProperties->Azimuth);
			UI::UIFloat::Draw("Inclination", &s_SkyboxProperties->Inclination);
		}

		if (ImGui::CollapsingHeader("Grid Settings"))
		{
			UI::UIBool::Draw("Draw Grid", &s_GridSettings->DrawGrid);

			if (s_GridSettings->DrawGrid)
			{
				UI::UIFloat::Draw("Inner Scale", &s_GridSettings->InnerScale);
				UI::UIFloat::Draw("Outer Scale", &s_GridSettings->OuterScale);
				UI::UIVector3::Draw("Grid Color", &s_GridSettings->GridColor);
			}
		}

		if (ImGui::CollapsingHeader("Bloom Settings"))
		{
			UI::UIFloat::Draw("Bloom Intensity", &s_BloomProperties->BloomIntensity);
			UI::UIFloat::Draw("Bloom Threshold", &s_BloomProperties->BloomThreshold);
			UI::UIFloat::Draw("Bloom Knee", &s_BloomProperties->BloomKnee);

			UI::UIBool::Draw("Bloom Dirt Enabled", &s_BloomProperties->BloomEnabled);
			if (s_BloomProperties->BloomEnabled)
				UI::UIFloat::Draw("Bloom Dirt Intensity", &s_BloomProperties->BloomDirtIntensity);
			else
				s_BloomProperties->BloomDirtIntensity = 0.0f;

			UI::UIBool::Draw("Display Compute Textures", &s_BloomProperties->DisplayBloomDebug);


			if (!s_BloomProperties->DisplayBloomDebug) return;
			glm::vec2 windowSize = { Application::GetApplication().GetWindow().GetWidth(), Application::GetApplication().GetWindow().GetHeight() };
			float aspect = (float)viewportSize.x / (float)viewportSize.y;
			ImGui::Image((ImTextureID)s_BloomProperties->BloomComputeTextures[0]->GetID(), { 300 * aspect, 300 }, { 0, 1 }, { 1, 0 });
			ImGui::Image((ImTextureID)s_BloomProperties->BloomComputeTextures[1]->GetID(), { 300 * aspect, 300 }, { 0, 1 }, { 1, 0 });
			ImGui::Image((ImTextureID)s_BloomProperties->BloomComputeTextures[2]->GetID(), { 300 * aspect, 300 }, { 0, 1 }, { 1, 0 });
		}
	}

	void SceneRenderer::InitializePipeline()
	{
		InitializeShadowPass();
		InitializeGeometryPass();
		InitializeSkybox();
		InitializeGrid();

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