#include "ohmpch.h"
#include "Ohm/Rendering/SceneRenderer.h"
#include "Ohm/Rendering/Renderer.h"
#include "Ohm/Rendering/Framebuffer.h"
#include "Ohm/Core/Application.h"
#include "Ohm/Rendering/Shader.h"
#include "Ohm/Rendering/RenderCommand.h"
#include "Ohm/Scene/Component.h"
#include "Ohm/UI/PropertyDrawer.h"

#include <glm/glm.hpp>
#include <imgui.h>

#include "EnvironmentMapPipeline.h"
#include "TextureLibrary.h"
#include "Ohm/Core/UUID.h"

namespace Ohm
{
	Ref<Scene> SceneRenderer::s_ActiveScene = nullptr;

	EditorCamera SceneRenderer::s_Camera(45.0f, 1920.0f/1080.0f, 0.1f, 1000.0f);

	Ref<RenderPass> SceneRenderer::s_GeometryPass = nullptr;
	Ref<RenderPass> SceneRenderer::s_SkyboxGeometryPass = nullptr;
	Ref<RenderPass> SceneRenderer::s_DebugDepthPass = nullptr;
	Ref<RenderPass> SceneRenderer::s_EnvironmentPass = nullptr;
	Ref<RenderPass> SceneRenderer::s_BloomPass = nullptr;
	Ref<RenderPass> SceneRenderer::s_SceneCompositePass = nullptr;

	Ref<SceneRenderer::SceneRenderProperties> SceneRenderer::s_SceneRenderProperties;
	Ref<SceneRenderer::BloomProperties> SceneRenderer::s_BloomProperties;

	float SceneRenderer::s_ImageViewerSizeFactor = .58f;
	glm::vec2 SceneRenderer::s_ViewportSize;

	void SceneRenderer::LoadScene(const Ref<Scene>& runtimeScene)
	{
		s_Camera.SetPosition({0.0f, 10.0f, 10.0f});
		s_ActiveScene = runtimeScene;
	}

	void SceneRenderer::UnloadScene()
	{
		s_ActiveScene = nullptr;
	}

	void SceneRenderer::InitializeUI()
	{
	}

	void SceneRenderer::InitializeGeometryPass()
	{
		FramebufferSpecification GeometryFBOSpec =
		{
			Application::GetApplication().GetWindow().GetWidth(), Application::GetApplication().GetWindow().GetHeight(),
			{ FramebufferTextureFormat::Depth, FramebufferTextureFormat::RGBA32F }
		};

		RenderPassSpecification GeometryRenderPassSpec;
		GeometryRenderPassSpec.Flags |= static_cast<uint32_t>(RenderFlag::DepthTest) | static_cast<uint32_t>(RenderFlag::Blend);
		GeometryRenderPassSpec.TargetFramebuffer = CreateRef<Framebuffer>(GeometryFBOSpec);
		s_GeometryPass = CreateRef<RenderPass>(GeometryRenderPassSpec);

		RenderPassSpecification SkyboxGeometryRenderPassSpec;
		SkyboxGeometryRenderPassSpec.Flags |= static_cast<uint32_t>(RenderFlag::DepthTest) | static_cast<uint32_t>(RenderFlag::Blend);
		SkyboxGeometryRenderPassSpec.TargetFramebuffer = GeometryRenderPassSpec.TargetFramebuffer;
		SkyboxGeometryRenderPassSpec.ClearColorFlag = SkyboxGeometryRenderPassSpec.ClearDepthFlag = false;
		SkyboxGeometryRenderPassSpec.PassMaterial = CreateRef<Material>("Skybox Materials", ShaderLibrary::Get("Skybox"));
		s_SkyboxGeometryPass = CreateRef<RenderPass>(SkyboxGeometryRenderPassSpec);
	}

	void SceneRenderer::InitializeDebugDepthPass()
	{
		FramebufferSpecification DebugDepthFBOSpec;
		DebugDepthFBOSpec.AttachmentSpecification = { FramebufferTextureFormat::RGBA32F };
		constexpr uint32_t ShadowMapResolution = 4096;
		DebugDepthFBOSpec.Width = DebugDepthFBOSpec.Height = ShadowMapResolution;

		RenderPassSpecification DebugDepthRenderPassSpec;
		DebugDepthRenderPassSpec.TargetFramebuffer = CreateRef<Framebuffer>(DebugDepthFBOSpec);
		DebugDepthRenderPassSpec.PassMaterial = CreateRef<Material>("Debug Depth Material", ShaderLibrary::Get("LinearDepthVisualizer"));
		
		s_DebugDepthPass = CreateRef<RenderPass>(DebugDepthRenderPassSpec);
	}

	void SceneRenderer::InitializeEnvironmentPass()
	{
		FramebufferSpecification fboSpec;
		fboSpec.AttachmentSpecification =
		{
			FramebufferTextureFormat::RGBA32F,
			FramebufferTextureFormat::RGBA32F,
			FramebufferTextureFormat::RGBA32F
		};
		fboSpec.Width = Application::GetApplication().GetWindow().GetWidth();
		fboSpec.Height = Application::GetApplication().GetWindow().GetHeight();

		Entity EnvironmentLightEntity = s_ActiveScene->GetEnvironmentLight();
		EnvironmentLightComponent& EnvironmentLight = EnvironmentLightEntity.GetComponent<EnvironmentLightComponent>();
		
		if(EnvironmentLight.Pipeline->GetSpecification().PipelineType == EnvironmentPipelineType::FromShader)
		{
			EnvironmentLight.Pipeline->GetSpecification().PreDispatchFn =
			[&EnvironmentLight](auto&& Unfiltered, auto&& Filtered)
			{
				const glm::vec3 TAI
				{
					EnvironmentLight.EnvironmentMapParams.Turbidity,
					EnvironmentLight.EnvironmentMapParams.Azimuth,
					EnvironmentLight.EnvironmentMapParams.Inclination
				};
				ShaderLibrary::Get("Preetham")->Bind();
				ShaderLibrary::Get("Preetham")->UploadUniformFloat3("u_TAI", TAI);
			};
			EnvironmentLight.Pipeline->GetSpecification().EnvironmentMapResolution = 1024;
			EnvironmentLight.Pipeline->GetSpecification().EnvironmentMapName = "Preetham Sky Model";
			EnvironmentLight.Pipeline->BuildFromShader("Preetham");
		}
		else if(EnvironmentLight.Pipeline->GetSpecification().PipelineType == EnvironmentPipelineType::FromFile)
			EnvironmentLight.Pipeline->BuildFromEquirectangularImage(EnvironmentLight.Pipeline->GetSpecification().FromFileFilePath);
		
		RenderPassSpecification EnvironmentPassSpec;
		EnvironmentPassSpec.TargetFramebuffer = CreateRef<Framebuffer>(fboSpec);
		EnvironmentPassSpec.PassMaterial = CreateRef<Material>("Skybox Material", ShaderLibrary::Get("Skybox"));
		EnvironmentPassSpec.Flags |= static_cast<uint32_t>(RenderFlag::DepthTest) | static_cast<uint32_t>(RenderFlag::Blend);
		
		s_EnvironmentPass = CreateRef<RenderPass>(EnvironmentPassSpec);
	}
	
	void SceneRenderer::InitializeBloomPass()
	{
		s_BloomProperties = CreateRef<BloomProperties>();
		s_BloomProperties->BloomShader = ShaderLibrary::Get("Bloom");
		Texture2DSpecification FileTextureSpec =
		{
			TextureUtils::WrapMode::Repeat,
			TextureUtils::WrapMode::Repeat,
			TextureUtils::FilterMode::Linear,
			TextureUtils::FilterMode::Linear,
			TextureUtils::ImageInternalFormat::FromImage,
			TextureUtils::ImageDataLayout::FromImage,
			TextureUtils::ImageDataType::UByte,
		};

		FileTextureSpec.Name = "Bloom Dirt Mask";
		const std::string DirtMaskPath = "assets/textures/dirt-mask.png";

		s_BloomProperties->BloomDirtTexture = TextureLibrary::LoadTexture2D(FileTextureSpec, DirtMaskPath);

		const glm::vec2 WindowDimensions = { Application::GetApplication().GetWindow().GetWidth(), Application::GetApplication().GetWindow().GetHeight() };
		uint32_t HalfWidth = WindowDimensions.x / 2;
		uint32_t HalfHeight = WindowDimensions.y / 2;
		HalfWidth += (s_BloomProperties->BloomWorkGroupSize - (HalfWidth % s_BloomProperties->BloomWorkGroupSize));
		HalfHeight += (s_BloomProperties->BloomWorkGroupSize - (HalfHeight % s_BloomProperties->BloomWorkGroupSize));

		Texture2DSpecification BloomTextureSpecification =
		{
			TextureUtils::WrapMode::ClampToEdge,
			TextureUtils::WrapMode::ClampToEdge,
			TextureUtils::FilterMode::LinearMipLinear,
			TextureUtils::FilterMode::Linear,
			TextureUtils::ImageInternalFormat::RGBA32F,
			TextureUtils::ImageDataLayout::RGBA,
			TextureUtils::ImageDataType::Float,
			HalfWidth, HalfHeight
		};

		s_BloomProperties->BloomComputeTextures.resize(3);
		BloomTextureSpecification.Name = "Bloom 1";
		s_BloomProperties->BloomComputeTextures[0] = CreateRef<Texture2D>(BloomTextureSpecification);
		BloomTextureSpecification.Name = "Bloom 2";
		s_BloomProperties->BloomComputeTextures[1] = CreateRef<Texture2D>(BloomTextureSpecification);
		BloomTextureSpecification.Name = "Bloom 3";
		s_BloomProperties->BloomComputeTextures[2] = CreateRef<Texture2D>(BloomTextureSpecification);
	}
	
	void SceneRenderer::InitializeSceneCompositePass()
	{
		s_SceneRenderProperties = CreateRef<SceneRenderProperties>();
		
		FramebufferSpecification CompositeFBOSpec;
		const auto& Window = Application::GetApplication().GetWindow();
		CompositeFBOSpec.AttachmentSpecification = { FramebufferTextureFormat::RGBA32F };
		CompositeFBOSpec.Width = Window.GetWidth();
		CompositeFBOSpec.Height = Window.GetHeight();

		RenderPassSpecification CompositeRenderPassSpec;
		CompositeRenderPassSpec.TargetFramebuffer = CreateRef<Framebuffer>(CompositeFBOSpec);
		CompositeRenderPassSpec.Flags |= static_cast<uint32_t>(RenderFlag::DepthTest) | static_cast<uint32_t>(RenderFlag::Blend);
		CompositeRenderPassSpec.PassMaterial = CreateRef<Material>("Scene Composite Material", ShaderLibrary::Get("SceneComposite"));

		s_SceneCompositePass = CreateRef<RenderPass>(CompositeRenderPassSpec);
	}

	void SceneRenderer::UploadPBRSamplers(const Ref<Material>& material)
	{
		Entity envLightEntity = s_ActiveScene->GetEnvironmentLight();
		const EnvironmentLightComponent& envLight = envLightEntity.GetComponent<EnvironmentLightComponent>();

		const uint32_t FilteredRadianceRendererID = TextureLibrary::GetCube(envLight.Pipeline->GetSpecification().GetFilteredCubeName())->GetID();
		const uint32_t IrradianceRendererID = TextureLibrary::GetCube(envLight.Pipeline->GetSpecification().GetIrradianceCubeName())->GetID();
		const uint32_t brdfLutId = TextureLibrary::Get2D("BRDF_LUT.png")->GetID();
		
		const TextureUniform radiance { FilteredRadianceRendererID, 5, 1 };
		const TextureUniform irradiance { IrradianceRendererID, 6, 1 };
		const TextureUniform brdf { brdfLutId, 7, 1 };
		material->Set<TextureUniform>("sampler_RadianceCube", radiance);
		material->Set<TextureUniform>("sampler_IrradianceCube", irradiance);
		material->Set<TextureUniform>("sampler_BRDFLUT", brdf);
	}
	
	void SceneRenderer::GeometryPass()
	{
		Renderer::BeginPass(s_GeometryPass);

		const auto primMeshView = s_ActiveScene->m_Registry.view<TransformComponent, PrimitiveRendererComponent>();
		for (const auto Entity : primMeshView)
		{
			auto [transform, primitive] = primMeshView.get<TransformComponent, PrimitiveRendererComponent>(Entity);
			
			if(primitive.PrimitiveType == Primitive::None) continue;
			
			UploadPBRSamplers(primitive.MaterialInstance);
			Renderer::UploadPerEntityData(transform);
			Renderer::DrawPrimitive(primitive);
		}
		EnvironmentLightComponent& EnvironmentLight = s_ActiveScene->GetEnvironmentLight().GetComponent<EnvironmentLightComponent>();
		const EnvironmentMapSpecification PipelineSpec = EnvironmentLight.Pipeline->GetSpecification();
		const uint32_t FilteredRadianceMapID = TextureLibrary::GetCube(PipelineSpec.GetFilteredCubeName())->GetID();
		const uint32_t UnfilteredRadianceMapID = TextureLibrary::GetCube(PipelineSpec.GetUnfilteredCubeName())->GetID();
		const uint32_t IrradianceMapID = TextureLibrary::GetCube(PipelineSpec.GetIrradianceCubeName())->GetID();

		const glm::mat4 ViewProjection = s_Camera.GetViewProjection();

		s_SkyboxGeometryPass->GetRenderPassSpecification().PassMaterial->Set<glm::vec3>("u_LODs", EnvironmentLight.EnvironmentMapSampleLODs);
		s_SkyboxGeometryPass->GetRenderPassSpecification().PassMaterial->Set<glm::vec3>("u_Intensities", EnvironmentLight.EnvironmentMapSampleIntensities);
		s_SkyboxGeometryPass->GetRenderPassSpecification().PassMaterial->Set<TextureUniform>("u_FilteredRadianceMap", {FilteredRadianceMapID, 0, 1});
		s_SkyboxGeometryPass->GetRenderPassSpecification().PassMaterial->Set<TextureUniform>("u_UnfilteredRadianceMap", {UnfilteredRadianceMapID, 1, 1});
		s_SkyboxGeometryPass->GetRenderPassSpecification().PassMaterial->Set<TextureUniform>("u_IrradianceMap", {IrradianceMapID, 2, 1});
		s_SkyboxGeometryPass->GetRenderPassSpecification().PassMaterial->Set<glm::mat4>("u_InverseViewProjection", glm::inverse(ViewProjection));
		Renderer::DrawSkybox(s_SkyboxGeometryPass->GetRenderPassSpecification().PassMaterial);

		Renderer::EndPass(s_SkyboxGeometryPass);
	}

	void SceneRenderer::DebugVisualizeDepthPass()
	{
		Renderer::BeginPass(s_DebugDepthPass);
		s_DebugDepthPass->GetRenderPassSpecification().TargetFramebuffer->Bind();
		s_DebugDepthPass->GetRenderPassSpecification().PassMaterial->Set<float>("u_Near", s_Camera.GetNearClip());
		s_DebugDepthPass->GetRenderPassSpecification().PassMaterial->Set<float>("u_Far", s_Camera.GetFarClip());
		const TextureUniform DepthTextureUniform { s_GeometryPass->GetRenderPassSpecification().TargetFramebuffer->GetDepthAttachmentID(), 0, 1 };
		s_DebugDepthPass->GetRenderPassSpecification().PassMaterial->Set<TextureUniform>("sampler_SceneDepth", DepthTextureUniform);
		Renderer::DrawFullScreenQuad(s_DebugDepthPass->GetRenderPassSpecification().PassMaterial);
		s_DebugDepthPass->GetRenderPassSpecification().TargetFramebuffer->Unbind();
		Renderer::EndPass(s_DebugDepthPass);
	}

	void SceneRenderer::EnvironmentPass()
	{
		EnvironmentLightComponent& EnvironmentLight = s_ActiveScene->GetEnvironmentLight().GetComponent<EnvironmentLightComponent>();
		if(EnvironmentLight.NeedsUpdate)
		{
			if(EnvironmentLight.Pipeline->GetSpecification().PipelineType == EnvironmentPipelineType::FromShader)
			{
				EnvironmentLight.Pipeline->GetSpecification().PreDispatchFn =
					[&EnvironmentLight](auto&& Unfiltered, auto&& Filtered)
					{
						const glm::vec3 TAI
						{
							EnvironmentLight.EnvironmentMapParams.Turbidity,
							EnvironmentLight.EnvironmentMapParams.Azimuth,
							EnvironmentLight.EnvironmentMapParams.Inclination
						};
						ShaderLibrary::Get("Preetham")->Bind();
						ShaderLibrary::Get("Preetham")->UploadUniformFloat3("u_TAI", TAI);
					};
				EnvironmentLight.Pipeline->GetSpecification().EnvironmentMapResolution = 1024;
				EnvironmentLight.Pipeline->GetSpecification().EnvironmentMapName = "Preetham Sky Model";
				EnvironmentLight.Pipeline->BuildFromShader("Preetham");
			}
			else if(EnvironmentLight.Pipeline->GetSpecification().PipelineType == EnvironmentPipelineType::FromFile)
				EnvironmentLight.Pipeline->BuildFromEquirectangularImage(EnvironmentLight.Pipeline->GetSpecification().FromFileFilePath);

			EnvironmentLight.NeedsUpdate = false;
		}
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
			s_GeometryPass->GetRenderPassSpecification().TargetFramebuffer->BindColorAttachment(0, 0);
			s_BloomProperties->BloomShader->UploadUniformInt("u_Texture", 0);
			s_BloomProperties->BloomComputeTextures[0]->BindToImageSlot(0, 0, TextureUtils::TextureAccessLevel::WriteOnly, TextureUtils::TextureShaderDataFormat::RGBA32F);

			s_BloomProperties->BloomShader->DispatchCompute(workGroupsX, workGroupsY, 1);
			s_BloomProperties->BloomShader->EnableShaderImageAccessBarrierBit();

			s_GeometryPass->GetRenderPassSpecification().TargetFramebuffer->UnbindColorAttachment(0, 0);
		}
		//------------------ PREFILTER -----------------//


		//------------------ DOWNSAMPLE -----------------//
		bloomConstants.Mode = 1;
		uint32_t mips = s_BloomProperties->BloomComputeTextures[0]->GetMipLevelCount() - 2;

		for (uint32_t mip = 1; mip < mips; mip++)
		{
			{
				auto [mipWidth, mipHeight] = s_BloomProperties->BloomComputeTextures[0]->GetMipSize(mip);
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

			auto [mipWidth, mipHeight] = s_BloomProperties->BloomComputeTextures[2]->GetMipSize(mips - 2);
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
				auto [mipWidth, mipHeight] = s_BloomProperties->BloomComputeTextures[2]->GetMipSize(mip);
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

		Texture2D::ClearBinding();
		s_BloomProperties->BloomShader->Unbind();
	}

	void SceneRenderer::SceneCompositePass()
	{
		Renderer::BeginPass(s_SceneCompositePass);
		const TextureUniform GeometryTexUniform {s_GeometryPass->GetRenderPassSpecification().TargetFramebuffer->GetColorAttachmentID(0), 0, 1};
		const TextureUniform BloomTextureUniform {s_BloomProperties->BloomComputeTextures[2]->GetID(), 1, 1};
		const TextureUniform BloomDirtTextureUniform {TextureLibrary::Get2D("Bloom Dirt Mask")->GetID(), 2, 1};
		
		s_SceneCompositePass->GetRenderPassSpecification().PassMaterial->Set<TextureUniform>("u_SceneTexture", GeometryTexUniform);
		s_SceneCompositePass->GetRenderPassSpecification().PassMaterial->Set<TextureUniform>("u_BloomTexture", BloomTextureUniform);
		s_SceneCompositePass->GetRenderPassSpecification().PassMaterial->Set<TextureUniform>("u_BloomDirtTexture", BloomDirtTextureUniform);

		s_SceneCompositePass->GetRenderPassSpecification().PassMaterial->Set<int>("u_BloomEnabled", s_BloomProperties->BloomEnabled ? 1 : 0);
		s_SceneCompositePass->GetRenderPassSpecification().PassMaterial->Set<float>("u_Exposure", s_SceneRenderProperties->Exposure);
		s_SceneCompositePass->GetRenderPassSpecification().PassMaterial->Set<float>("u_BloomIntensity", s_BloomProperties->BloomIntensity);
		s_SceneCompositePass->GetRenderPassSpecification().PassMaterial->Set<float>("u_BloomDirtIntensity", s_BloomProperties->BloomDirtIntensity);
		Renderer::DrawFullScreenQuad(s_SceneCompositePass->GetRenderPassSpecification().PassMaterial);
		Renderer::EndPass(s_SceneCompositePass);
	}
	
	void SceneRenderer::InitializePipeline()
	{
		InitializeGeometryPass();
		InitializeDebugDepthPass();
		InitializeEnvironmentPass();
		InitializeBloomPass();
		InitializeSceneCompositePass();
	}

	void SceneRenderer::SubmitPipeline()
	{
		s_ActiveScene->UpdateLightingEnvironment(s_Camera);
		Renderer::BeginScene(s_ActiveScene, s_Camera);
		DebugVisualizeDepthPass();
		EnvironmentPass();
		GeometryPass();

		BloomPass();
		SceneCompositePass();
		Renderer::EndScene();
	}

	void SceneRenderer::UpdateCamera(float deltaTime)
	{
		s_Camera.Update(deltaTime);
	}

	void SceneRenderer::OnEvent(Event& e)
	{
		s_Camera.OnEvent(e);
	}

	void SceneRenderer::ValidateResize(glm::vec2 viewportSize)
	{
		if(s_ViewportSize == viewportSize) return;
		s_ViewportSize = viewportSize;

		s_GeometryPass->GetRenderPassSpecification().TargetFramebuffer->Resize((uint32_t)s_ViewportSize.x, (uint32_t)s_ViewportSize.y);
		s_EnvironmentPass->GetRenderPassSpecification().TargetFramebuffer->Resize((uint32_t)s_ViewportSize.x, (uint32_t)s_ViewportSize.y);
		s_SceneCompositePass->GetRenderPassSpecification().TargetFramebuffer->Resize((uint32_t)s_ViewportSize.x, (uint32_t)s_ViewportSize.y);
		s_DebugDepthPass->GetRenderPassSpecification().TargetFramebuffer->Resize((uint32_t)s_ViewportSize.x, (uint32_t)s_ViewportSize.y);

		uint32_t halfWidth = s_ViewportSize.x / 2;
		uint32_t halfHeight = s_ViewportSize.y / 2;
		halfWidth += (s_BloomProperties->BloomWorkGroupSize - (halfWidth % s_BloomProperties->BloomWorkGroupSize));
		halfHeight += (s_BloomProperties->BloomWorkGroupSize - (halfHeight % s_BloomProperties->BloomWorkGroupSize));
				
		s_BloomProperties->BloomComputeTextures[0]->Resize(halfWidth, halfHeight);
		s_BloomProperties->BloomComputeTextures[1]->Resize(halfWidth, halfHeight);
		s_BloomProperties->BloomComputeTextures[2]->Resize(halfWidth, halfHeight);

		s_Camera.SetViewportSize(s_ViewportSize.x, s_ViewportSize.y);
	}

	void SceneRenderer::DrawTextureViewerUI()
	{
	    constexpr float BaseGuessWindowHeight = 125.0f;
	    constexpr float MinImageWidth = 120.0f;
	    constexpr float MaxImageWidth = 480.0f;
	    constexpr float MinImageHeight = 67.5f;
	    constexpr float MaxImageHeight = 270.0f;
	    constexpr uint32_t ImagesPerAxis = 3;

	    constexpr float MinWindowWidth = MinImageWidth * ImagesPerAxis;
	    constexpr float MaxWindowWidth = MaxImageWidth * ImagesPerAxis;
	    constexpr float MinWindowHeight = MinImageHeight * ImagesPerAxis + BaseGuessWindowHeight;
	    constexpr float MaxWindowHeight = MaxImageHeight * ImagesPerAxis + BaseGuessWindowHeight;
	    
	    float ImageWidth = MinImageWidth * (1.0f - s_ImageViewerSizeFactor) + MaxImageWidth * s_ImageViewerSizeFactor;
	    float ImageHeight = MinImageHeight * (1.0f - s_ImageViewerSizeFactor) + MaxImageHeight * s_ImageViewerSizeFactor;
	    float WindowWidth =  MinWindowWidth * (1.0f - s_ImageViewerSizeFactor) + MaxWindowWidth * s_ImageViewerSizeFactor;
	    float WindowHeight =  MinWindowHeight * (1.0f - s_ImageViewerSizeFactor) + MaxWindowHeight * s_ImageViewerSizeFactor;

	    ImGui::SetNextWindowSize({WindowWidth, WindowHeight});
	    ImGui::Begin("Texture Viewer", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
	    ImGui::DragFloat("Image Size Factor", &s_ImageViewerSizeFactor, 0.01f, 0.0, 1.0);

	    std::vector<std::string> ImageToolTips =
	    {
	    	"Filtered Sky Radiance",
	        "Unfiltered Sky Radiance",
	        "Irradiance",
	    	"Scene Geometry",
	        "Depth",
	    	"Bloom",
	    };

	    if (ImGui::BeginTable("Texture Viewer", 3))
	    {
	        for (int row = 0; row < ImagesPerAxis + 1; row++)
	        {
	            ImGui::TableNextRow();
	            for (int column = 0; column < ImagesPerAxis; column++)
	            {
	                const int Index = column + ImagesPerAxis * row;
	                uint32_t ImageID = 0;
	                switch(Index)
	                {
						case 0: ImageID = s_EnvironmentPass->GetRenderPassSpecification().TargetFramebuffer->GetColorAttachmentID(0);	break; // Filtered Sky Radiance
	                	case 1: ImageID = s_EnvironmentPass->GetRenderPassSpecification().TargetFramebuffer->GetColorAttachmentID(1);	break; // Unfiltered Sky Radiance
	                	case 2: ImageID = s_EnvironmentPass->GetRenderPassSpecification().TargetFramebuffer->GetColorAttachmentID(2);	break; // Irradiance 
	                    case 3: ImageID = s_GeometryPass->GetRenderPassSpecification().TargetFramebuffer->GetColorAttachmentID(0);		break; // PBR Geometry Output
	                    case 4: ImageID = s_DebugDepthPass->GetRenderPassSpecification().TargetFramebuffer->GetColorAttachmentID(0);	break; // Depth
	                    case 6: ImageID = s_BloomProperties->BloomComputeTextures[2]->GetID();												break; // Bloom
	                    default: ;
	                }
	                ImGui::TableSetColumnIndex(column);
	            	const std::string TextureName = Index <= ImageToolTips.size() - 1 ? ImageToolTips[Index] : "";
	                ImGui::Text(TextureName.c_str());

	            	bool SaveSuccess = false;
	            	Ref<Framebuffer> SaveFBO = nullptr;

	            	std::string FileSaveName;
	                if(ImGui::ImageButton(reinterpret_cast<ImTextureID>(ImageID), {ImageWidth, ImageHeight}, {0, 1}, {1, 0}))
	            	{
		                uint32_t AttachmentIndex = 0;
		                switch(Index)
	            		{
	            			case 0: SaveFBO = s_EnvironmentPass->GetRenderPassSpecification().TargetFramebuffer; AttachmentIndex = 0;			break; // Filtered Sky Radiance
	            			case 1: SaveFBO = s_EnvironmentPass->GetRenderPassSpecification().TargetFramebuffer; AttachmentIndex = 1;			break; // Unfiltered Sky Radiance
	            			case 2: SaveFBO = s_EnvironmentPass->GetRenderPassSpecification().TargetFramebuffer; AttachmentIndex = 2;			break; // Irradiance 
	            			case 3: SaveFBO = s_GeometryPass->GetRenderPassSpecification().TargetFramebuffer; AttachmentIndex = 0;				break; // PBR Geometry Output
	            			case 5: SaveFBO = s_DebugDepthPass->GetRenderPassSpecification().TargetFramebuffer;	AttachmentIndex = 0;			break; // Depth
	            		}

	            		if(SaveFBO != nullptr)
	            		{
	            			static std::string DirPath = "assets/image-saves/";
	            			FileSaveName = TextureName + "-(" + std::to_string(UUID()) + ").exr";
	            			SaveSuccess = SaveFBO->SaveAttachmentAsEXR(DirPath + FileSaveName, AttachmentIndex);
	            		}
	            	}

	            	if(SaveFBO != nullptr)
	            	{
	            		std::string Message = SaveSuccess ? "Successfully saved EXR with name " + FileSaveName + "!" :  "Failed to save EXR with name " + FileSaveName + "!";
	            		OHM_CORE_INFO(Message);
	            	}
	            }
	        }
	        ImGui::EndTable();
	    }

	    ImGui::End();
	}

	void SceneRenderer::DrawSceneRendererUI(const glm::vec2 ViewportSize)
	{
		if (ImGui::CollapsingHeader("Scene Render Settings"))
		{
			UI::UIFloat::Draw("Exposure", &s_SceneRenderProperties->Exposure);
			UI::UIBool::Draw("Apply Color Correction", &s_SceneRenderProperties->ApplyColorCorrection);
		}
		
		if (ImGui::CollapsingHeader("Bloom Settings"))
		{
			UI::UIBool::Draw("Bloom Enabled", &s_BloomProperties->BloomEnabled);

			if(s_BloomProperties->BloomEnabled)
			{
				UI::UIFloat::Draw("Bloom Intensity", &s_BloomProperties->BloomIntensity);
				UI::UIFloat::Draw("Bloom Threshold", &s_BloomProperties->BloomThreshold);
				UI::UIFloat::Draw("Bloom Knee", &s_BloomProperties->BloomKnee);

				UI::UIBool::Draw("Bloom Dirt Enabled", &s_BloomProperties->BloomDirtEnabled);
				if (s_BloomProperties->BloomEnabled)
					UI::UIFloat::Draw("Bloom Dirt Intensity", &s_BloomProperties->BloomDirtIntensity);
				else
					s_BloomProperties->BloomDirtIntensity = 0.0f;

				UI::UIBool::Draw("Display Compute Textures", &s_BloomProperties->DisplayBloomDebug);

				if (s_BloomProperties->DisplayBloomDebug)
				{
					float aspect = static_cast<float>(ViewportSize.x) / static_cast<float>(ViewportSize.y);
					ImGui::Image(reinterpret_cast<ImTextureID>(s_BloomProperties->BloomComputeTextures[0]->GetID()), { 300 * aspect, 300 }, { 0, 1 }, { 1, 0 });
					ImGui::Image(reinterpret_cast<ImTextureID>(s_BloomProperties->BloomComputeTextures[1]->GetID()), { 300 * aspect, 300 }, { 0, 1 }, { 1, 0 });
					ImGui::Image(reinterpret_cast<ImTextureID>(s_BloomProperties->BloomComputeTextures[2]->GetID()), { 300 * aspect, 300 }, { 0, 1 }, { 1, 0 });
				}
			}
		}
		
		if(ImGui::CollapsingHeader("Scene Information"))
		{
			UI::UIVector3::Draw("Camera Position", &s_Camera.GetPosition());
			float nearClip = s_Camera.GetNearClip();
			float farClip = s_Camera.GetFarClip();
			UI::UIFloat::Draw("Near Clip", &nearClip);
			UI::UIFloat::Draw("Far Clip", &farClip);

			if(nearClip != s_Camera.GetNearClip() || farClip != s_Camera.GetFarClip())
				s_Camera.SetNearFarClip(nearClip, farClip);
		}
	}

	const Ref<Framebuffer>& SceneRenderer::GetSceneCompositeFBO()
	{
		return s_SceneCompositePass->GetRenderPassSpecification().TargetFramebuffer;
	}
}
