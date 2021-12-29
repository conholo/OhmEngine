#pragma once

#include "Ohm/Scene/Scene.h"
#include "Ohm/Scene/Entity.h"
#include "Ohm/Rendering/RenderPass.h"
#include "Ohm/Rendering/EditorCamera.h"
#include "Ohm/Rendering/Texture2D.h"
#include "Ohm/Rendering/TextureCube.h"
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
		static void InitializeSkybox();
		static void InitializeGrid();

		static void InitializeBloomPass();
		static void InitializeCompositePass();

		static void ShadowPass();
		static void GeometryPass();
		static void DrawSkybox();
		static void DrawGrid();

		static void BloomPass();
		static void CompositePass();

	private:
		struct SceneRenderProperties
		{
			float Exposure = 1.0f;
		};
		static Ref<SceneRenderProperties> s_SceneRenderProperties;

		struct BloomProperties
		{
			Ref<Shader> BloomShader;
			std::vector<Ref<Texture2D>> BloomComputeTextures;
			Ref<Texture2D> BloomDirtTexture;
			const uint32_t BloomWorkGroupSize = 4;

			bool DisplayBloomDebug = false;
			bool BloomEnabled = true;
			bool BloomDirtEnabled = true;
			float BloomThreshold = 1.5f;
			float BloomKnee = 0.220f;
			float BloomIntensity = 0.790f;
			float BloomDirtIntensity = 0.0f;
		};
		static Ref<BloomProperties> s_BloomProperties;

		struct GridData
		{
			Ref<Shader> GridShader;
			Ref<Mesh> FullScreenQuad;
		};

		static Ref<GridData> s_GridData;

		struct GridSettings
		{
			bool DrawGrid = true;
			float InnerScale = 1.0f;
			float OuterScale = 0.1f;
			glm::vec3 GridColor = {0.08f, 0.08f, 0.08f};
		};

		static Ref<GridSettings> s_GridSettings;

	private:
		static Ref<Mesh> s_EnvironmentCube;
		static Ref<TextureCube> s_EnvironmentMap;
		static Ref<Shader> s_EnvironmentMapShader;
		static Ref<Shader> s_SkyboxShader;

		struct SkyboxProperties
		{
			float Intensity = 1.0f;
		    uint32_t LOD = 0;

			float Turbidity = 2.5f;
			float Azimuth = 2.5f;
			float Inclination = 4.5f;
		};
		static Ref<SkyboxProperties> s_SkyboxProperties;

	private:
		static Ref<Scene> s_ActiveScene;
		static EditorCamera s_EditorCamera;

		static Ref<RenderPass> s_GeometryPass;
		static Ref<RenderPass> s_ShadowPass;
		static Ref<RenderPass> s_SceneCompositePass;

		static Ref<UniformBuffer> s_ShadowUniformbuffer;
		static Ref<Shader> s_DebugDepthShader;

		static glm::ivec2 m_ViewportSize;
	};
}