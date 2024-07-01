#pragma once

#include "Ohm.h"
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
		void OnUpdate(float deltaTime) override;
		void OnDetach() override;
		void OnUIRender() override;
		void OnEvent(Event& event) override;

	private:

		Entity m_Quad;
		Entity m_Sphere;
		Entity m_Cube;
		Entity m_Plane;
		Entity m_DirectionalLight;
		Ref<Scene> m_Scene;

		ConsolePanel m_ConsolePanel;
		UI::Viewport m_ViewportPanel;
		UI::SceneHierarchyPanel m_SceneHierarchyPanel;

		Ref<Material> m_EngineGeometryMaterial;
	};
}