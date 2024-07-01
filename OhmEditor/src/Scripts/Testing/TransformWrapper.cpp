#include "TransformWrapper.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Ohm
{

	TransformWrapper::TransformWrapper(TransformComponent& transform)
		:m_Transform(transform)
	{

	}

	TransformWrapper::TransformWrapper(TransformComponent& transform, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
		:m_Transform(transform)
	{
		m_Transform.Translation = position;
		m_Transform.RotationDegrees = rotation;
		m_Transform.Scale = scale;
	}

	void TransformWrapper::Translate(const glm::vec3& direction, float speed)
	{
		m_Transform.Translation += direction * speed;
	}
	
	glm::vec3 TransformWrapper::TransformInversePoint(const glm::vec3& point)
	{
		return m_Transform.Transform() * glm::vec4(point, 1.0);
	}

	glm::vec3 TransformWrapper::TransformPoint(const glm::vec3& point)
	{
		return glm::inverse(m_Transform.Transform()) * glm::vec4(point, 1.0);
	}

	glm::vec3 TransformWrapper::TransformInverseDirection(const glm::vec3& direction)
	{
		return m_Transform.Transform() * glm::vec4(direction, 0.0);
	}

	glm::vec3 TransformWrapper::TransformDirection(const glm::vec3& direction)
	{
		return glm::inverse(m_Transform.Transform()) * glm::vec4(direction, 0.0);
	}

	glm::vec3 TransformWrapper::Up()
	{
		return glm::rotate(CalculateOrientation(), { 0.0f, 1.0f, 0.0f });
	}

	glm::vec3 TransformWrapper::Right()
	{
		return glm::rotate(CalculateOrientation(), { 1.0f, 0.0f, 0.0f });
	}

	glm::vec3 TransformWrapper::Forward()
	{
		return glm::rotate(CalculateOrientation(), { 0.0f, 0.0f, 1.0f });
	}

	glm::quat TransformWrapper::CalculateOrientation()
	{
		auto& rotation = m_Transform.RotationDegrees;
		return glm::quat(rotation);
	}
}