#pragma once


#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "Ohm/Core/Memory.h"
#include "Ohm/Rendering/Shader.h"

namespace Ohm
{
	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string & tag)
			: Tag(tag){ }
	};

	struct TransformComponent
	{
		glm::vec3 Translation{ 0.0f };
		glm::vec3 Rotation{ 0.0f };
		glm::vec3 Scale{ 1.0f };


		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation)
			:Translation(translation) {}

		TransformComponent(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale)
			:Translation(translation), Rotation(rotation), Scale(scale) { }

		glm::mat4 Transform() const
		{
			glm::mat4 translation = glm::translate(glm::mat4(1.0f), Translation);
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));
			glm::mat4 scale = glm::scale(glm::mat4(1.0f), Scale);

			return translation * rotation * scale;
		}
	};

	struct MeshRendererComponent
	{
		// TODO:: Make Mesh Class

		// TODO:: Make Material Class
		glm::vec4 Color{ 1.0f };
		Ref<Shader> MaterialShader;


		// TODO:: Make Vertex/Geometry Data Class/Struct
		float* Vertices;
		uint32_t VertexCount;

		uint32_t* Indices;
		uint32_t IndexCount;

		MeshRendererComponent() = default;
		MeshRendererComponent(const MeshRendererComponent&) = default;
		MeshRendererComponent(const Ref<Shader>& shader, const glm::vec4& color, float* vertices, uint32_t vertexCount, uint32_t* indices, uint32_t indexCount)
			:MaterialShader(shader), Color(color), Vertices(vertices), VertexCount(vertexCount), Indices(indices), IndexCount(indexCount){ }
	};

	struct CameraComponent
	{
		glm::mat4 View;
		glm::mat4 Projection;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
		CameraComponent(const glm::mat4& view, const glm::mat4& projection)
			:View(view), Projection(projection) { }
	};
}