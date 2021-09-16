#pragma once

#include "Ohm/Scene/Scene.h"
#include "Ohm/Scene/Entity.h"
#include "Ohm/Rendering/RenderPass.h"
#include "Ohm/Rendering/EditorCamera.h"
#include "Ohm/Core/Time.h"
#include "Ohm/Event/Event.h"

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
	private:
		static void InitializePreShadowPass();
		static void InitializeGeometryPass();

		static void PreShadowPass();
		static void GeometryPass();

	private:
		static Ref<Scene> s_ActiveScene;
		static EditorCamera s_EditorCamera;

		static Ref<RenderPass> s_GeometryPass;
		static Ref<RenderPass> s_PreShadowPass;

		static Ref<UniformBuffer> s_ShadowUniformbuffer;
		static Ref<Shader> s_DebugDepthShader;
	};
}