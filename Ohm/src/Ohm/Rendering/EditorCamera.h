#pragma once

#include "Ohm/Event/Event.h"
#include "Ohm/Core/Time.h"

#include <glm/glm.hpp>


namespace Ohm
{
	class EditorCamera
	{
	public:

		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

		void Update(Time dt);
		void OnEvent(Event& event);

		void SetViewportSize(uint32_t width, uint32_t height);

		float GetZDistance() const { return m_DistanceFromFocalPoint; }
		void SetZDistance(float zDistance) { m_DistanceFromFocalPoint = zDistance; }

		glm::mat4 GetProjectionView() const { return m_ProjectionMatrix * m_ViewMatrix; }

		glm::vec3 Forward() const;
		glm::vec3 Up() const;
		glm::vec3 Right() const;
		glm::vec3 Position() const { return m_Position; }

	private:
		glm::quat CalculateOrientation() const;
		glm::vec3 CalculatePosition() const;
		void RecalculateView();
		void RecalculateProjection();

	private:

		float m_Speed = 5.0f;

		float m_FOV = 45.0f;
		float m_AspectRatio;

		float m_NearClip = 0.1f;
		float m_FarClip = 1000.0f;

		float m_Pitch = 0.0f, m_Yaw = 0.0f;
		float m_PitchDelta = 0.0f, m_YawDelta = 0.0f;

		float m_DistanceFromFocalPoint = 10.0f;
		float m_Rotation;

		glm::vec3 m_WorldRotationEulers{ 0.0f };
		glm::vec3 m_Position { 0.0f };

		glm::vec3 m_PositionDelta{ 0.0f };
		glm::vec3 m_FocalPoint { 0.0f };

		glm::vec2 m_CurrentMousePosition{ 0.0f };

		glm::mat4 m_ViewMatrix { 1.0f };
		glm::mat4 m_ProjectionMatrix { 1.0f };
	};
}