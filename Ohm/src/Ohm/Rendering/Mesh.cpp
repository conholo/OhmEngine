#include "ohmpch.h"
#include "Ohm/Rendering/Mesh.h"

namespace Ohm
{
	Ref<Mesh> Mesh::CreatePrimitive(Primitive primitive)
	{
		switch (primitive)
		{
			case Ohm::Primitive::Quad: return Quad();
			case Ohm::Primitive::Cube: return Cube();
			default: break;
		}
	}

	Ref<Mesh> Mesh::Quad()
	{
		std::vector<Vertex> vertices =
		{
			Vertex{ {-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f} },
			Vertex{ { 0.5f, -0.5f, 0.0f}, {1.0f, 0.0f} },
			Vertex{ { 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f} },
			Vertex{ {-0.5f,  0.5f, 0.0f}, {0.0f, 1.0f} },
		};

		std::vector<uint32_t> indices =
		{
			 0, 1, 2, 2, 3, 0
		};

		return CreateRef<Mesh>(vertices, indices);
	}


	Ref<Mesh> Mesh::Cube()
	{
		std::vector<Vertex> vertices =
		{
			Vertex{ {-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f} },
			Vertex{ { 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f} },
			Vertex{ { 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f} },
			Vertex{ {-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f} },

			Vertex{ {-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f} },
			Vertex{ { 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f} },
			Vertex{ { 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f} },
			Vertex{ {-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f} }
		};

		std::vector<uint32_t> indices =
		{
			// front
			0, 1, 2,
			2, 3, 0,
			// right
			1, 5, 6,
			6, 2, 1,
			// back
			7, 6, 5,
			5, 4, 7,
			// left
			4, 0, 3,
			3, 7, 4,
			// bottom
			4, 5, 1,
			1, 0, 4,
			// top
			3, 2, 6,
			6, 7, 3
		};

		return CreateRef<Mesh>(vertices, indices);
	}
}