#pragma once
#include <glm/glm.hpp>

namespace Ohm
{
	struct Vertex
	{
		glm::vec3 Position {0.0f};
		glm::vec3 Normal {0.0f};
		glm::vec3 Tangent {0.0f};
		glm::vec3 Binormal {0.0f};
		glm::vec2 TexCoord {0.0f};
	};
}