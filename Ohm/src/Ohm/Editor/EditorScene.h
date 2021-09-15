#pragma once

#include "Ohm/Scene/Scene.h"
#include "Ohm/Scene/Entity.h"
#include "Ohm/Rendering/RenderPass.h"
#include "Ohm/Rendering/EditorCamera.h"
#include "Ohm/Core/Time.h"
#include "Ohm/Event/Event.h"

namespace Ohm
{
	class EditorScene
	{
	public:

		static void LoadScene(const Ref<Scene>& runtimeScene);
		static void UnloadScene();

		static void UpdateEditorCamera(Time dt);

		static void InitializePipeline();
		static void ExecutePipeline();
		static void OnEvent(Event& e);

		static Ref<Framebuffer> GetMainColorBuffer();
		static Ref<Framebuffer> GetDepthBuffer();

		static void ValidateResize(glm::vec2 viewportSize);
	private:
		static void InitializeDepthPass();
		static void InitializeGeometryPass();

		static void DepthPass();
		static void GeometryPass();

	private:
		static Ref<Scene> s_ActiveScene;
		static EditorCamera s_EditorCamera;

		static Ref<RenderPass> s_GeometryPass;
		static Ref<RenderPass> s_PreDepthPass;

		static Ref<UniformBuffer> s_ShadowUniformbuffer;
		static Ref<Shader> s_DebugDepthShader;
	};
}