#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace Ohm
{
	struct Vertex
	{
		glm::vec3 VertexPosition;
		glm::vec2 TexCoords;
		glm::vec3 Normals;
	};
}