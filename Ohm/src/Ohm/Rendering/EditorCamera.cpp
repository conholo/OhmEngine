#include "ohmpch.h"
#include "Ohm/Rendering/EditorCamera.h"
#include "Ohm/Event/WindowEvent.h"
#include "Ohm/Core/Input.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Ohm
{

	EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
		:m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip)
	{
		m_Position = CalculatePosition();

		glm::quat orientation = CalculateOrientation();
		m_WorldRotationEulers = glm::degrees(glm::eulerAngles(orientation));

		m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);

		RecalculateProjection();
	}

	glm::vec3 EditorCamera::CalculatePosition() const
	{
		return m_FocalPoint - Forward() * m_DistanceFromFocalPoint + m_PositionDelta;
	}


	glm::quat EditorCamera::CalculateOrientation() const
	{
		return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
	}

	void EditorCamera::RecalculateView()
	{
		const float yawSign = Up().y < 0.0f ? -1.0f : 1.0f;

		const glm::vec3 lookAt = m_Position + Forward();
		m_WorldRotationEulers = glm::normalize(m_FocalPoint - m_Position);
		m_FocalPoint = m_Position + Forward() * m_DistanceFromFocalPoint;
		m_DistanceFromFocalPoint = glm::distance(m_Position, m_FocalPoint);
		m_ViewMatrix = glm::lookAt(m_Position, lookAt, glm::vec3{ 0.0f, yawSign, 0.0f });

		m_YawDelta *= 0.6f;
		m_PitchDelta *= 0.6f;
		m_PositionDelta *= 0.8f;
	}

	void EditorCamera::RecalculateProjection()
	{
		m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
	}

	void EditorCamera::Update(Time dt)
	{
		const glm::vec2 mousePosition = Input::GetMousePosition();
		const glm::vec2 mouseDelta = (mousePosition - m_CurrentMousePosition) * 0.002f;


		if (Input::IsMouseButtonPressed(2))
		{
			const float yawSign = Up().y < 0.0f ? -1.0f : 1.0f;
			m_YawDelta += yawSign * mouseDelta.x * m_Speed * dt.Seconds();
			m_PitchDelta += mouseDelta.y * m_Speed * dt.Seconds();
		}

		if (Input::IsKeyPressed(Key::A))
			m_PositionDelta -= Right() * m_Speed * dt.Seconds();
		if(Input::IsKeyPressed(Key::D))
			m_PositionDelta += Right() * m_Speed * dt.Seconds();
		if (Input::IsKeyPressed(Key::W))
			m_PositionDelta += Up() * m_Speed * dt.Seconds();
		if (Input::IsKeyPressed(Key::S))
			m_PositionDelta -= Up() * m_Speed * dt.Seconds();


		m_Position += m_PositionDelta;
		m_Yaw += m_YawDelta;
		m_Pitch += m_PitchDelta;

		m_CurrentMousePosition = mousePosition;
		CalculatePosition();
		RecalculateView();
	}

	void EditorCamera::OnEvent(Event& event)
	{
	}

	void EditorCamera::SetViewportSize(uint32_t width, uint32_t height)
	{
		m_AspectRatio = width / height;
		RecalculateProjection();
	}

	glm::vec3 EditorCamera::Forward() const
	{
		return glm::rotate(CalculateOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	}

	glm::vec3 EditorCamera::Up() const
	{
		return glm::rotate(CalculateOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 EditorCamera::Right() const
	{
		return glm::rotate(CalculateOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	}
}