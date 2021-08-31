#include "ohmpch.h"
#include "Ohm/Rendering/Mesh.h"
#include <glad/glad.h>

namespace Ohm
{
#define PI 3.14159265359

	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
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
			case Ohm::Primitive::Quad: return Quad();
			case Ohm::Primitive::Cube: return Cube();
			case Ohm::Primitive::Sphere: return Sphere(1.0f, 36, 18);
			default: break;
		}
	}

	Ref<Mesh> Mesh::Quad()
	{
		// FIX THIS 
		std::vector<Vertex> vertices =
		{
			Vertex{ {-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
			Vertex{ { 0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
			Vertex{ { 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f} },
			Vertex{ {-0.5f,  0.5f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f} },
		};

		std::vector<uint32_t> indices =
		{
			 0, 1, 2, 2, 3, 0
		};

		return CreateRef<Mesh>(vertices, indices);
	}


	Ref<Mesh> Mesh::Cube()
	{

		// LAYOUT: Position (3F) - TexCoords (2F) - Normals (3F)
		std::vector<Vertex> verticesSix =
		{
			// Front Face
			Vertex{ {-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}, { 0.0f,   0.0f,  1.0f } },	// 0 0 
			Vertex{ { 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}, { 0.0f,   0.0f,  1.0f } },	// 1 1
			Vertex{ { 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}, { 0.0f,   0.0f,  1.0f } },	// 2 2
			Vertex{ {-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}, { 0.0f,   0.0f,  1.0f } },	// 3 3
															
			// Right Face									
			Vertex{ { 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}, { 1.0f,   0.0f,  0.0f } },	// 1 4 
			Vertex{ { 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}, { 1.0f,   0.0f,  0.0f } },	// 5 5
			Vertex{ { 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}, { 1.0f,   0.0f,  0.0f } },	// 6 6
			Vertex{ { 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}, { 1.0f,   0.0f,  0.0f } },	// 2 7

			// Back Face									
			Vertex{ {-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, { 0.0f,   0.0f, -1.0f } },	// 4 8
			Vertex{ { 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}, { 0.0f,   0.0f, -1.0f } },	// 5 9
			Vertex{ { 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}, { 0.0f,   0.0f, -1.0f } },	// 6 10
			Vertex{ {-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}, { 0.0f,   0.0f, -1.0f } },	// 7 11
															
			// Left Face									
			Vertex{ {-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}, { -1.0f,  0.0f,  0.0f } },	// 0 12
			Vertex{ {-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}, { -1.0f,  0.0f,  0.0f } },	// 4 13
			Vertex{ {-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}, { -1.0f,  0.0f,  0.0f } },	// 7 14
			Vertex{ {-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}, { -1.0f,  0.0f,  0.0f } },	// 3 15
															
			// Bottom Face									
			Vertex{ {-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}, {  0.0f, -1.0f,  0.0f } },	// 0 16
			Vertex{ { 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}, {  0.0f, -1.0f,  0.0f } },	// 1 17
			Vertex{ { 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}, {  0.0f, -1.0f,  0.0f } },	// 5 18
			Vertex{ {-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}, {  0.0f, -1.0f,  0.0f } },	// 4 19
															
			// Top Face										
			Vertex{ {-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}, {  0.0f,  1.0f,  0.0f } },	// 3 20
			Vertex{ { 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}, {  0.0f,  1.0f,  0.0f } },	// 2 21
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

		return CreateRef<Mesh>(verticesSix, indicesSix);
	}

	Ref<Mesh> Mesh::Sphere(float radius, uint32_t sectorCount, uint32_t stackCount)
	{
		if (radius <= 0)
			radius = 0.1f;

		std::vector<Vertex> vertices;

		glm::vec3 vertexPosition(0.0f);
		glm::vec3 normal(0.0f);
		glm::vec2 texCoords(0.0f);

		float xy;
		float lengthInverse = 1 / radius;

		float sectorStep = 2 * PI / sectorCount;
		float stackStep = PI / stackCount;
		float sectorAngle, stackAngle;

		for (uint32_t i = 0; i <= stackCount; i++)
		{
			stackAngle = PI / 2 - i * stackStep;
			xy = radius * glm::cos(stackAngle);
			vertexPosition.z = radius * glm::sin(stackAngle);

			for (uint32_t j = 0; j <= sectorCount; j++)
			{
				sectorAngle = j * sectorStep;

				vertexPosition.x = xy * glm::cos(sectorAngle);
				vertexPosition.y = xy * glm::sin(sectorAngle);

				normal.x = vertexPosition.x * lengthInverse;
				normal.y = vertexPosition.y * lengthInverse;
				normal.z = vertexPosition.z * lengthInverse;

				texCoords.x = (float)j / sectorCount;
				texCoords.y = (float)i / stackCount;

				vertices.push_back({ vertexPosition, texCoords, normal });
			}
		}

		std::vector<uint32_t> indices;

		int k1, k2;

		for (uint32_t i = 0; i < stackCount; i++)
		{
			k1 = i * (sectorCount + 1);
			k2 = k1 + sectorCount + 1;

			for (uint32_t j = 0; j < sectorCount; j++, k1++, k2++)
			{
				if (i != 0)
				{
					indices.push_back(k1);
					indices.push_back(k2);
					indices.push_back(k1 + 1);
				}

				if (i != stackCount - 1)
				{
					indices.push_back(k1 + 1);
					indices.push_back(k2);
					indices.push_back(k2 + 1);
				}
			}
		}

		return CreateRef<Mesh>(vertices, indices);
	}

}