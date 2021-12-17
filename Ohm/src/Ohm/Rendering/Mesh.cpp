#include "ohmpch.h"
#include "Ohm/Rendering/Mesh.h"
#include <glad/glad.h>

namespace Ohm
{
#define PI 3.14159265359

	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, Primitive primitive)
		:m_Vertices(vertices), m_Indices(indices), m_PrimitiveType(primitive)
	{
		CreateRenderPrimitives(vertices, indices);
	}

	void Mesh::Bind() const
	{
		m_VertexBuffer->Bind();
		m_IndexBuffer->Bind();
	}

	void Mesh::Unbind() const
	{
		m_VertexBuffer->Unbind();
		m_IndexBuffer->Unbind();
	}

	void Mesh::CreateRenderPrimitives(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
	{
		float* basePtr = &vertices.data()->VertexPosition.x;
		m_VertexBuffer = CreateRef<VertexBuffer>(basePtr, sizeof(Vertex) * vertices.size());

		BufferLayout layout
		(
			{
				{ "a_Position", ShaderDataType::Float3 },
				{ "a_TexCoord", ShaderDataType::Float2 },
				{ "a_Normal",  ShaderDataType::Float3 },
			}
		);
		m_VertexBuffer->SetLayout(layout);

		uint32_t* indexPtr = indices.data();
		m_IndexBuffer = CreateRef<IndexBuffer>(indexPtr, indices.size());
	}

	Ref<Mesh> Mesh::CreatePrimitive(Primitive primitive)
	{
		switch (primitive)
		{
			case Ohm::Primitive::Quad:				return Quad();
			case Ohm::Primitive::FullScreenQuad:	return FullScreenQuad();
			case Ohm::Primitive::Cube:				return Cube();
			case Ohm::Primitive::Plane:				return Plane();
			case Ohm::Primitive::Sphere:			return Sphere(1.0f);
			default: break;
		}

		return nullptr;
	}

	Ref<Mesh> Mesh::Quad()
	{
		// LAYOUT: Position (3F) - TexCoords (2F) - Normals (3F)
		std::vector<Vertex> vertices =
		{
			Vertex{ {-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f} },
			Vertex{ { 0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f} },
			Vertex{ { 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f} },
			Vertex{ {-0.5f,  0.5f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f} },
		};

		std::vector<uint32_t> indices =
		{
			 0, 1, 2, 2, 3, 0
		};

		return CreateRef<Mesh>(vertices, indices, Primitive::Quad);
	}

	Ref<Mesh> Mesh::FullScreenQuad()
	{
		// LAYOUT: Position (3F) - TexCoords (2F) - Normals (3F)
		std::vector<Vertex> vertices =
		{
			Vertex{ {-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f} },
			Vertex{ { 1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f} },
			Vertex{ { 1.0f,  1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f} },
			Vertex{ {-1.0f,  1.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f} },
		};

		std::vector<uint32_t> indices =
		{
			 0, 1, 2, 2, 3, 0
		};

		return CreateRef<Mesh>(vertices, indices, Primitive::Quad);
	}

	Ref<Mesh> Mesh::Plane()
	{
		// LAYOUT: Position (3F) - TexCoords (2F) - Normals (3F)
		std::vector<Vertex> vertices =
		{
			Vertex{ {-0.5f,	 0.0f,  0.5f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} },
			Vertex{ { 0.5f,  0.0f,  0.5f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f} },
			Vertex{ { 0.5f,  0.0f, -0.5f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f} },
			Vertex{ {-0.5f,  0.0f, -0.5f}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f} },
		};

		std::vector<uint32_t> indices =
		{
			 0, 1, 2, 2, 3, 0
		};

		return CreateRef<Mesh>(vertices, indices, Primitive::Plane);
	}



	Ref<Mesh> Mesh::Cube()
	{

		// LAYOUT: Position (3F) - TexCoords (2F) - Normals (3F)
		std::vector<Vertex> verticesSix =
		{
			// Front Face
			Vertex{ {-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}, { 0.0f,   0.0f,  1.0f } },	// 0 0 
			Vertex{ { 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}, { 0.0f,   0.0f,  1.0f } },	// 1 1
			Vertex{ { 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}, { 0.0f,   0.0f,  1.0f } },	// 2 2
			Vertex{ {-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}, { 0.0f,   0.0f,  1.0f } },	// 3 3
															
			// Right Face									
			Vertex{ { 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}, { 1.0f,   0.0f,  0.0f } },	// 1 4 
			Vertex{ { 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}, { 1.0f,   0.0f,  0.0f } },	// 5 5
			Vertex{ { 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}, { 1.0f,   0.0f,  0.0f } },	// 6 6
			Vertex{ { 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}, { 1.0f,   0.0f,  0.0f } },	// 2 7

			// Back Face									
			Vertex{ {-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, { 0.0f,   0.0f, -1.0f } },	// 4 8
			Vertex{ { 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}, { 0.0f,   0.0f, -1.0f } },	// 5 9
			Vertex{ { 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}, { 0.0f,   0.0f, -1.0f } },	// 6 10
			Vertex{ {-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}, { 0.0f,   0.0f, -1.0f } },	// 7 11
															
			// Left Face									
			Vertex{ {-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}, { -1.0f,  0.0f,  0.0f } },	// 0 12
			Vertex{ {-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}, { -1.0f,  0.0f,  0.0f } },	// 4 13
			Vertex{ {-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}, { -1.0f,  0.0f,  0.0f } },	// 7 14
			Vertex{ {-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}, { -1.0f,  0.0f,  0.0f } },	// 3 15
															
			// Bottom Face									
			Vertex{ {-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}, {  0.0f, -1.0f,  0.0f } },	// 0 16
			Vertex{ { 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}, {  0.0f, -1.0f,  0.0f } },	// 1 17
			Vertex{ { 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}, {  0.0f, -1.0f,  0.0f } },	// 5 18
			Vertex{ {-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}, {  0.0f, -1.0f,  0.0f } },	// 4 19
															
			// Top Face										
			Vertex{ {-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}, {  0.0f,  1.0f,  0.0f } },	// 3 20
			Vertex{ { 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f}, {  0.0f,  1.0f,  0.0f } },	// 2 21
			Vertex{ { 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}, {  0.0f,  1.0f,  0.0f } },	// 6 22
			Vertex{ {-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}, {  0.0f,  1.0f,  0.0f } }	// 7 23
		};

		std::vector<uint32_t> indicesSix =
		{
			// front
			0, 1, 2,
			2, 3, 0,
			// right
			4, 5, 6,
			6, 7, 4,
			// back
			8, 9, 10,
			10, 11, 8,
			// left
			12, 13, 14,
			14, 15, 12,
			// bottom
			16, 17, 18,
			18, 19, 16,
			// top
			20, 21, 22,
			22, 23, 20
		};

		return CreateRef<Mesh>(verticesSix, indicesSix, Primitive::Cube);
	}

	Ref<Mesh> Mesh::Sphere(float radius)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		constexpr float latitudeBands = 30;
		constexpr float longitudeBands = 30;

		for (float latitude = 0.0f; latitude <= latitudeBands; latitude++)
		{
			const float theta = latitude * (float)PI / latitudeBands;
			const float sinTheta = glm::sin(theta);
			const float cosTheta = glm::cos(theta);

			float texT = 1.0f - theta / PI;

			for (float longitude = 0.0f; longitude <= longitudeBands; longitude++)
			{
				const float phi = longitude * 2.0f * (float)PI / longitudeBands;
				const float sinPhi = glm::sin(phi);
				const float cosPhi = glm::cos(phi);

				float texS = 1.0f - (phi / (2 * PI));

				Vertex vertex;
				vertex.Normals = { cosPhi * sinTheta, cosTheta, sinPhi * sinTheta };
				vertex.TexCoords = { texS, texT };
				vertex.VertexPosition = { radius * vertex.Normals.x, radius * vertex.Normals.y, radius * vertex.Normals.z };
				vertices.push_back(vertex);
			}
		}

		for (uint32_t latitude = 0; latitude < (uint32_t)latitudeBands; latitude++)
		{
			for (uint32_t longitude = 0; longitude < (uint32_t)longitudeBands; longitude++)
			{
				const uint32_t first = (latitude * ((uint32_t)longitudeBands + 1)) + longitude;
				const uint32_t second = first + (uint32_t)longitudeBands + 1;

				indices.push_back(first);
				indices.push_back(first + 1);
				indices.push_back(second);

				indices.push_back(second);
				indices.push_back(first + 1);
				indices.push_back(second + 1);
			}
		}

		return CreateRef<Mesh>(vertices, indices, Primitive::Sphere);
	}

	std::string Mesh::PrimitiveToString(Primitive primitive)
	{
		switch (primitive)
		{
			case Primitive::Cube:	return "Cube";
			case Primitive::Sphere: return "Sphere";
			case Primitive::Quad:	return "Quad";
			case Primitive::Plane:	return "Plane";
		}

		return "";
	}

}