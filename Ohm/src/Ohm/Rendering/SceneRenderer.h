#pragma once

#include "Ohm/Scene/Scene.h"
#include "Ohm/Scene/Entity.h"
#include "Ohm/Rendering/RenderPass.h"
#include "Ohm/Rendering/EditorCamera.h"
#include "Ohm/Event/Event.h"
#include "Ohm/Rendering/Texture2D.h"

namespace Ohm
{
	class SceneRenderer
	{
	public:
		static void LoadScene(const Ref<Scene>& runtimeScene);
		static void UnloadScene();
		
		static void UpdateCamera(float deltaTime);

		static void InitializePipeline();
		static void SubmitPipeline();
		static void OnEvent(Event& e);

		static void ValidateResize(glm::vec2 viewportSize);
		static void DrawTextureViewerUI();
		static void DrawSceneRendererUI(const glm::vec2 ViewportSize);
		static const Ref<Framebuffer>& GetSceneCompositeFBO();
		static EditorCamera& GetCamera() { return s_Camera;}

	private:
		static void InitializeUI();

		static void UploadPBRSamplers(const Ref<Material>& material);
		
		static void InitializeGeometryPass();
		static void InitializeDebugDepthPass();
		static void InitializeEnvironmentPass();
		static void InitializeBloomPass();
		static void InitializeSceneCompositePass();

		static void GeometryPass();
		static void DebugVisualizeDepthPass();
		static void EnvironmentPass();
		static void BloomPass();
		static void SceneCompositePass();

	private:
		static Ref<Scene> s_ActiveScene;
		static EditorCamera s_Camera;

		//static Ref<RenderPass> s_ShadowPass;
		static Ref<RenderPass> s_GeometryPass;
		static Ref<RenderPass> s_SkyboxGeometryPass;
		static Ref<RenderPass> s_DebugDepthPass;
		static Ref<RenderPass> s_EnvironmentPass;
		static Ref<RenderPass> s_BloomPass;
		static Ref<RenderPass> s_SceneCompositePass;

		struct SceneRenderProperties
		{
			float Exposure = 1.0f;
			bool ApplyColorCorrection = true;
		};
		static Ref<SceneRenderProperties> s_SceneRenderProperties;

		struct BloomProperties
		{
			Ref<Shader> BloomShader{};
			std::vector<Ref<Texture2D>> BloomComputeTextures{};
			Ref<Texture2D> BloomDirtTexture{};
			const uint32_t BloomWorkGroupSize = 4;

			bool DisplayBloomDebug = false;
			bool BloomEnabled = true;
			bool BloomDirtEnabled = true;
			float BloomThreshold = 2.0f;
			float BloomKnee = 0.220f;
			float BloomIntensity = 0.2f;
			float BloomDirtIntensity = 0.1f;
		};
		static Ref<BloomProperties> s_BloomProperties;

		struct Benchmarks
		{
			float LastEnvironmentMappingPassTime = 0.0f;
			float LastPPTransmittancePassTime = 0.0;
			float LastSkyCompositionPassTime = 0.0;
		};
		static glm::vec2 s_ViewportSize;
		static float s_ImageViewerSizeFactor;
	};
}