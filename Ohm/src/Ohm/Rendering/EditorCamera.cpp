#include "ohmpch.h"
#include "Ohm/Rendering/EditorCamera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Ohm
{

	EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
		:m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip)
	{
		RecalculateView();
		RecalculateProjection();
	}

	void EditorCamera::Update()
	{

	}

	void EditorCamera::OnEvent(Event& event)
	{

	}

	void EditorCamera::SetViewportSize(uint32_t width, uint32_t height)
	{

	}

	glm::vec3 EditorCamera::Forward() const
	{
		return glm::vec3(0.0f);
	}

	glm::vec3 EditorCamera::Up() const
	{
		return glm::vec3(0.0f);
	}

	glm::vec3 EditorCamera::Right() const
	{
		return glm::vec3(0.0f);
	}

	glm::quat EditorCamera::Orientation() const
	{
		return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
	}

	void EditorCamera::RecalculatePosition()
	{
	}

	void EditorCamera::RecalculateView()
	{

	}

	void EditorCamera::RecalculateProjection()
	{

	}
}