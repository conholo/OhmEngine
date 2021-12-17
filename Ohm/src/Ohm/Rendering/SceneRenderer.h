#pragma once

#include "Ohm/Scene/Scene.h"
#include "Ohm/Scene/Entity.h"
#include "Ohm/Rendering/RenderPass.h"
#include "Ohm/Rendering/EditorCamera.h"
#include "Ohm/Rendering/Texture2D.h"
#include "Ohm/Core/Time.h"
#include "Ohm/Event/Event.h"
#include "Ohm/UI/PropertyDrawer.h"

namespace Ohm
{
	class SceneRenderer
	{
	public:

		static void LoadScene(const Ref<Scene>& runtimeScene);
		static void UnloadScene();

		static void UpdateEditorCamera(float deltaTime);

		static void InitializePipeline();
		static void SubmitPipeline();
		static void OnEvent(Event& e);

		static Ref<Framebuffer> GetMainColorBuffer();
		static Ref<Framebuffer> GetDepthBuffer();

		static void ValidateResize(glm::vec2 viewportSize);

		static void DrawSceneRendererUI(const glm::vec2 viewportSize);

	private:

		static void InitializeUI();

		static void InitializeShadowPass();
		static void InitializeGeometryPass();

		static void InitializeBloomPass();
		static void InitializeCompositePass();

		static void ShadowPass();
		static void GeometryPass();

		static void BloomPass();
		static void CompositePass();

	private:
		struct BloomProperties
		{
			Ref<Shader> BloomShader;
			std::vector<Ref<Texture2D>> BloomComputeTextures;
			Ref<Texture2D> BloomDirtTexture;
			const uint32_t BloomWorkGroupSize = 4;

			bool DisplayBloomDebug = false;
			bool BloomEnabled = true;
			bool BloomDirtEnabled = true;
			float BloomThreshold = 20.0f;
			float BloomKnee = 0.220f;
			float BloomIntensity = 0.790f;
			float BloomDirtIntensity = 1.0f;
		};
		static Ref<BloomProperties> s_BloomProperties;

	private:
		static Ref<Scene> s_ActiveScene;
		static EditorCamera s_EditorCamera;

		static Ref<RenderPass> s_GeometryPass;
		static Ref<RenderPass> s_ShadowPass;
		static Ref<RenderPass> s_SceneCompositePass;

		static Ref<UniformBuffer> s_ShadowUniformbuffer;
		static Ref<Shader> s_DebugDepthShader;

		static glm::ivec2 m_ViewportSize;

		struct UIPropertyDrawers
		{
			float Exposure = 1.0f;
			Ref<UI::UIFloat> ExposureDrawer = CreateRef<UI::UIFloat>("Exposure", &Exposure);
			Ref<UI::UIFloat> BloomIntensityDrawer;
			Ref<UI::UIFloat> BloomKneeDrawer;
			Ref<UI::UIFloat> BloomThresholdDrawer;
			Ref<UI::UIFloat> BloomDirtIntensityDrawer;
		};

		static Ref<UIPropertyDrawers> s_Drawers;
	};
}