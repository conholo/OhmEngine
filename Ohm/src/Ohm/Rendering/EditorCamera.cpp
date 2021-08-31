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


		if (Input::IsMouseButtonPressed(2) || Input::IsMouseButtonPressed(1))
		{
			if (Input::IsKeyPressed(Key::LeftAlt))
			{
				// Reset Rotation
				m_Yaw = 0.0f;
				m_Pitch = 0.0f;
			}
			else
			{
				// Fly forward/back
				if (Input::IsKeyPressed(Key::W))
					m_PositionDelta += Forward() * m_PanSpeed * dt.Seconds();
				if (Input::IsKeyPressed(Key::S))
					m_PositionDelta -= Forward() * m_PanSpeed * dt.Seconds();

				// Pitch/Yaw adjustment from mouse pan
				const float yawSign = Up().y < 0.0f ? -1.0f : 1.0f;
				m_YawDelta += yawSign * mouseDelta.x * m_RotationSpeed * dt.Seconds();
				m_PitchDelta += mouseDelta.y * m_RotationSpeed * dt.Seconds();
			}
		}
		else
		{
			// Fly up/down
			if (Input::IsKeyPressed(Key::W))
				m_PositionDelta += Up() * m_PanSpeed * dt.Seconds();
			if (Input::IsKeyPressed(Key::S))
				m_PositionDelta -= Up() * m_PanSpeed * dt.Seconds();
		}

		// Fly left/right
		if (Input::IsKeyPressed(Key::A))
			m_PositionDelta -= Right() * m_PanSpeed * dt.Seconds();
		if(Input::IsKeyPressed(Key::D))
			m_PositionDelta += Right() * m_PanSpeed * dt.Seconds();


		m_Position += m_PositionDelta;
		m_Yaw += m_YawDelta;
		m_Pitch += m_PitchDelta;

		m_CurrentMousePosition = mousePosition;
		RecalculateView();
	}

	void EditorCamera::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);

		dispatcher.Dispatch<MouseScrolledEvent>(OHM_BIND_FN(EditorCamera::OnScroll));
	}

	bool EditorCamera::OnScroll(MouseScrolledEvent& event)
	{
		m_DistanceFromFocalPoint -= event.GetYOffset();
		m_Position = m_FocalPoint - Forward() * m_DistanceFromFocalPoint;

		if (m_DistanceFromFocalPoint > 1.0f)
		{
			m_FocalPoint += Forward();
			m_DistanceFromFocalPoint = 1.0f;
		}

		m_PositionDelta += event.GetYOffset() * Forward();

		RecalculateView();
		return true;
	}

	void EditorCamera::SetViewportSize(float width, float height)
	{
		m_AspectRatio = width / height;
		OHM_INFO(m_AspectRatio);
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