#pragma once

#include "Ohm.h"

#include <glm/glm.hpp>

namespace Ohm
{
	class TransformWrapper
	{
	public:

		TransformWrapper() = default;
		TransformWrapper(TransformWrapper&) = default;
		TransformWrapper(TransformComponent& transform);
		TransformWrapper(TransformComponent& transform, const glm::vec3 & position, const glm::vec3 & rotation, const glm::vec3 & scale);

		void Translate(const glm::vec3& direction, float speed);
		void SetPosition(const glm::vec3& position) { m_Transform.Translation = position; }
		void SetRotation(const glm::vec3& rotation) { m_Transform.Rotation = rotation; }
		void SetScale(const glm::vec3& scale) { m_Transform.Scale = scale; }

		const glm::vec3& GetPosition() const { return m_Transform.Translation; }
		const glm::vec3& GetRotation() const { return m_Transform.Rotation; }
		const glm::vec3& GetScale() const { return m_Transform.Scale; }

		glm::vec3& GetPosition() { return m_Transform.Translation; }
		glm::vec3& GetRotation() { return m_Transform.Rotation; }
		glm::vec3& GetScale() { return m_Transform.Scale; }

		// To world space.
		glm::vec3 TransformInversePoint(const glm::vec3& point);
		// To local space.
		glm::vec3 TransformPoint(const glm::vec3& point);
		// To world space.
		glm::vec3 TransformInverseDirection(const glm::vec3& direction);
		// To local space.
		glm::vec3 TransformDirection(const glm::vec3& direction);

		glm::vec3 Up();
		glm::vec3 Right();
		glm::vec3 Forward();

	private:
		glm::quat CalculateOrientation();

	private:
		TransformComponent& m_Transform;
	};
}
