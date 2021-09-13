#pragma once

#include "Ohm.h"
#include "Ohm/Rendering/EditorCamera.h"

#include "Scripts/Testing/TransformWrapper.h"
#include "Panels/ConsolePanel.h"
#include "Panels/Viewport.h"
#include "Panels/SceneHierarchyPanel.h"

namespace Ohm
{
	class EditorLayer : public Layer
	{
	public:

		EditorLayer();
		virtual ~EditorLayer();

		void OnAttach() override;
		void OnUpdate(Time dt) override;
		void OnDetach() override;
		void OnUIRender() override;
		void OnEvent(Event& event) override;

	private:
		bool OnKeyPressed(KeyPressedEvent& event);

	private:

		Entity m_Quad;
		Entity m_Sphere;
		Entity m_Cube;
		Entity m_Plane;
		Entity m_DirectionalLight;
		Ref<TransformWrapper> m_CubeWrapper;
		Ref<TransformWrapper> m_SphereWrapper;
		Ref<TransformWrapper> m_PlaneWrapper;
		Ref<TransformWrapper> m_QuadWrapper;
		Ref<TransformWrapper> m_LightDemoWrapper;





		EditorCamera m_Camera;
		glm::vec4 m_ClearColor{ 0.1f, 0.1f, 0.1f, 1.0f };


		Ref<Scene> m_Scene;
		Ref<Framebuffer> m_Framebuffer;

		ConsolePanel m_ConsolePanel;
		Viewport m_ViewportPanel;
		SceneHierarchyPanel m_SceneHierarchyPanel;

		float m_ElapsedTime = 0.0f;
	};
}