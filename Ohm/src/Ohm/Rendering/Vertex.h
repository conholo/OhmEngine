#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace Ohm
{
	struct Vertex
	{
		glm::vec3 VertexPosition;
		glm::vec2 TexCoords;
	};

	struct GeometryData
	{
		GeometryData() = default;
		GeometryData(const std::initializer_list<Vertex>& vertices, const std::initializer_list<uint32_t>& indices)
			:Vertices(vertices), Indices(indices) { }

		std::vector<Vertex> Vertices;
		std::vector<uint32_t> Indices;
	};
}