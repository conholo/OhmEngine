#pragma once

#include "Ohm/Event/Event.h"

#include <glm/glm.hpp>


namespace Ohm
{
	class EditorCamera
	{
	public:

		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

		void Update();
		void OnEvent(Event& event);

		void SetViewportSize(uint32_t width, uint32_t height);

		float GetZDistance() const { return m_ZDistance; }
		void SetZDistance(float zDistance) { m_ZDistance = zDistance; }

		glm::mat4 GetProjectionView() const { return m_ProjectionMatrix * m_ViewMatrix; }

		glm::vec3 Forward() const;
		glm::vec3 Up() const;
		glm::vec3 Right() const;
		glm::quat Orientation() const;
		glm::vec3 Position() const { return m_Position; }

	private:

		void RecalculatePosition();
		void RecalculateView();
		void RecalculateProjection();

	private:
		float m_FOV = 45.0f;
		float m_AspectRatio;

		float m_NearClip = 0.1f;
		float m_FarClip = 1000.0f;

		float m_Pitch = 0.0f, m_Yaw = 0.0f;

		float m_ZDistance = 10.0f;
		float m_Rotation;
		glm::vec3 m_Position { 0.0f };
		glm::vec3 m_FocalPosition = glm::vec3(0, 0, 10.0f);

		glm::mat4 m_ViewMatrix{ 1.0f };
		glm::mat4 m_ProjectionMatrix{ 1.0f };
	};
}