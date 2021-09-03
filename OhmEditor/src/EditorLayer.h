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
		bool OnWindowResized(WindowResizedEvent& windowResizedEvent);

	private:

		Entity m_Sphere;
		Entity m_Cube;
		Entity m_DirectionalLight;

		glm::vec3 m_SpherePosition{ 0.0f, 2.5f, 0.0f };
		glm::vec3 m_SphereSize{ 1.0f };
		glm::vec4 m_SphereColor{ 1.0f };

		glm::vec3 m_CubePosition{ 0.0f, 0.0f, 0.0f };
		glm::vec3 m_CubeRotationDegrees{ 45.0f };
		glm::vec3 m_CubeRotation{ 0.0f };
		glm::vec3 m_CubeSize{ 2.0f };
		glm::vec4 m_CubeColor{ 1.0f };

		bool m_LightSpin = false;
		glm::vec3 m_LightPosition{ 0.0f, 10.0f, 0.0f };
		glm::vec3 m_LightSize{ 0.5f };
		glm::vec3 m_LightRotationDegrees{ 0.0f };
		glm::vec3 m_LightRotation{ 0.0f };

		glm::vec3 m_PlanePosition{ 0.0f, -2.0f, 0.0f };
		glm::vec3 m_PlaneSize{ 20.0f, 0.01f, 20.0f };
		glm::vec4 m_PlaneColor{ 1.0f };

		EditorCamera m_Camera;

		Ref<Scene> m_Scene;
		float m_ElapsedTime = 0.0f;
	};
}