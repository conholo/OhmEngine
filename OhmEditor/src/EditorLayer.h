#pragma once

#include "Ohm.h"
#include "Ohm/Rendering/EditorCamera.h"

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

		glm::vec3 m_QuadPosition{ 0.0f, 0.0f, 0.0f };
		glm::vec3 m_QuadSize{ 1.0f, 1.0f, 1.0f };

		glm::vec3 m_PlanePosition{ 0.0f, -2.0f, 0.0f };
		glm::vec3 m_PlaneSize{ 5.0f, 1.0f, 5.0f };

		EditorCamera m_Camera;
		Ref<Scene> m_Scene;
	};
}