#include "ohmpch.h"
#include "Ohm/Rendering/Mesh.h"
#include <glad/glad.h>

namespace Ohm
{
#define PI 3.14159265359

	Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, Primitive primitive)
		: m_PrimitiveType(primitive), m_Vertices(vertices), m_Indices(indices)
	{
		CreateRenderPrimitives();
	}

	static void CalculateTandBTriangle(std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
	{
		for(uint32_t TriangleIndex = 0; TriangleIndex < indices.size(); TriangleIndex += 3)
		{
			const uint32_t IndexA = indices[TriangleIndex];
			const uint32_t IndexB = indices[TriangleIndex + 1];
			const uint32_t IndexC = indices[TriangleIndex + 2];
			Vertex& A = vertices[IndexA];
			Vertex& B = vertices[IndexB];
			Vertex& C = vertices[IndexC];

			const glm::vec3 edge1 = B.Position - A.Position;
			const glm::vec3 edge2 = C.Position - A.Position;
			const glm::vec2 deltaUV1 = B.TexCoord - A.TexCoord;
			const glm::vec2 deltaUV2 = C.TexCoord - A.TexCoord;
			const float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

			glm::vec3 Tangent;
			Tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
			Tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
			Tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

			glm::vec3 Binormal;
			Binormal.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
			Binormal.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
			Binormal.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

			A.Tangent = B.Tangent = C.Tangent = Tangent;
			A.Binormal = B.Binormal = C.Binormal = Binormal;
		}
	}
	
	void Mesh::Bind() const
	{
		m_VertexArray->Bind();
		m_VertexBuffer->Bind();
		m_IndexBuffer->Bind();
	}

	void Mesh::Unbind() const
	{
		m_VertexArray->Unbind();
		m_VertexBuffer->Unbind();
		m_IndexBuffer->Unbind();
	}

	void Mesh::CreateRenderPrimitives()
	{
		m_VertexArray = CreateRef<VertexArray>();
		float* baseVertexPtr = &m_Vertices.data()->Position.x;
		m_VertexBuffer = CreateRef<VertexBuffer>(baseVertexPtr, m_Vertices.size() * sizeof(Vertex));

		const BufferLayout layout = BufferLayout
		(
			{
				{ "a_Position", ShaderDataType::Float3 },
				{ "a_Normal",	ShaderDataType::Float3 },
				{ "a_Tangent", ShaderDataType::Float3 },
				{ "a_Binormal", ShaderDataType::Float3 },
				{ "a_TexCoord", ShaderDataType::Float2 }
			}
		);
		m_VertexBuffer->SetLayout(layout);

		m_IndexBuffer = CreateRef<IndexBuffer>(m_Indices.data(), m_Indices.size());
		m_VertexArray->EnableVertexAttributes(m_VertexBuffer);
		m_VertexArray->SetIndexBuffer(m_IndexBuffer);
	}

	Ref<Mesh> MeshFactory::Create(Primitive primitiveType)
	{
		switch (primitiveType)
		{
			case Primitive::Plane:					return Plane();
			case Primitive::Quad:					return Quad();
			case Primitive::FullScreenQuad:			return FullScreenQuad();
			case Primitive::Triangle:				return Triangle();
			case Primitive::Cube:					return Cube();
			case Primitive::Sphere:					return Sphere(1.0f);
			case Primitive::TessellatedQuad:		return TessellatedQuad(10);
			case Primitive::Skybox:					return Skybox();
			case Primitive::Icosphere:				return Icosphere(5, 1.0f);
		default: ;
		}

		return nullptr;
	}

	Ref<Mesh> MeshFactory::Plane()
	{
		std::vector vertices =
		{
			Vertex{ {-0.5f,  0.0f,  0.5f},  {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
			Vertex{ { 0.5f,  0.0f,  0.5f},  {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
			Vertex{ { 0.5f,  0.0f, -0.5f},  {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
			Vertex{ {-0.5f,  0.0f, -0.5f},  {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },
		};

		std::vector<uint32_t> indices =
		{
			0, 1, 2, 2, 3, 0
	   };
		CalculateTandBTriangle(vertices, indices);
		
		return CreateRef<Mesh>(vertices, indices, Primitive::Plane);
	}

	Ref<Mesh> MeshFactory::Triangle()
	{
		std::vector<Vertex> vertices =
		{
			// Front Face
			Vertex{ {-0.5f, -0.5f,  0.0f}, { 0.0f, 0.0f, 1.0f }, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },	// 0 0 
			Vertex{ { 0.5f, -0.5f,  0.0f}, { 0.0f, 0.0f, 1.0f }, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },	// 1 1
			Vertex{ { 0.0f,  0.5f,  0.0f}, { 0.0f, 0.0f, 1.0f }, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.5f, 1.0f} },	// 2 2
		};

		std::vector<uint32_t> indices = { 0, 1, 2 };

		return CreateRef<Mesh>(vertices, indices, Primitive::Triangle);
	}

	Ref<Mesh> MeshFactory::Quad()
	{
		std::vector<Vertex> vertices =
		{
			Vertex{ {-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
			Vertex{ { 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
			Vertex{ { 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
			Vertex{ {-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },
		};

		std::vector<uint32_t> indices =
		{
			0, 1, 2, 2, 3, 0
	   };

		return CreateRef<Mesh>(vertices, indices, Primitive::Quad);
	}

	Ref<Mesh> MeshFactory::FullScreenQuad()
	{
		std::vector<Vertex> vertices =
		{
			Vertex{ {-1.0f, -1.0f, 0.0f},  {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
			Vertex{ { 1.0f, -1.0f, 0.0f},  {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
			Vertex{ { 1.0f,  1.0f, 0.0f},  {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
			Vertex{ {-1.0f,  1.0f, 0.0f},  {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },
		};

		std::vector<uint32_t> indices =
		{
			0, 1, 2, 2, 3, 0
	   };

		return CreateRef<Mesh>(vertices, indices, Primitive::FullScreenQuad);
	}


	Ref<Mesh> MeshFactory::Cube()
	{
		std::vector<Vertex> vertices =
		{
			// Front Face
			Vertex{ {-0.5f, -0.5f,  0.5f}, { 0.0f,   0.0f,  1.0f }, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}  },	// 0 0 
			Vertex{ { 0.5f, -0.5f,  0.5f}, { 0.0f,   0.0f,  1.0f }, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}  },	// 1 1
			Vertex{ { 0.5f,  0.5f,  0.5f}, { 0.0f,   0.0f,  1.0f }, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}  },	// 2 2
			Vertex{ {-0.5f,  0.5f,  0.5f}, { 0.0f,   0.0f,  1.0f }, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}  },	// 3 3

			// Right Face									
			Vertex{ { 0.5f, -0.5f,  0.5f}, { 1.0f,   0.0f,  0.0f }, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}  },	// 1 4 
			Vertex{ { 0.5f, -0.5f, -0.5f}, { 1.0f,   0.0f,  0.0f }, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}  },	// 5 5
			Vertex{ { 0.5f,  0.5f, -0.5f}, { 1.0f,   0.0f,  0.0f }, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}  },	// 6 6
			Vertex{ { 0.5f,  0.5f,  0.5f}, { 1.0f,   0.0f,  0.0f }, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}  },	// 2 7

			// Back Face									
			Vertex{ { 0.5f, -0.5f, -0.5f},  { 0.0f,   0.0f, -1.0f }, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },	// 4 8
			Vertex{ {-0.5f, -0.5f, -0.5f},  { 0.0f,   0.0f, -1.0f }, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },	// 5 9
			Vertex{ {-0.5f,  0.5f, -0.5f},  { 0.0f,   0.0f, -1.0f }, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },	// 6 10
			Vertex{ { 0.5f,  0.5f, -0.5f},  { 0.0f,   0.0f, -1.0f }, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },	// 7 11

			// Left Face									
			Vertex{ {-0.5f, -0.5f, -0.5f},  { -1.0f,  0.0f,  0.0f }, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },	// 0 12
			Vertex{ {-0.5f, -0.5f,  0.5f},  { -1.0f,  0.0f,  0.0f }, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },	// 4 13
			Vertex{ {-0.5f,  0.5f,  0.5f},  { -1.0f,  0.0f,  0.0f }, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },	// 7 14
			Vertex{ {-0.5f,  0.5f, -0.5f},  { -1.0f,  0.0f,  0.0f }, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },	// 3 15

			// Bottom Face									
			Vertex{ { 0.5f, -0.5f,  0.5f},  {  0.0f, -1.0f,  0.0f }, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },	// 0 16
			Vertex{ {-0.5f, -0.5f,  0.5f},  {  0.0f, -1.0f,  0.0f }, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },	// 1 17
			Vertex{ {-0.5f, -0.5f, -0.5f},  {  0.0f, -1.0f,  0.0f }, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },	// 5 18
			Vertex{ { 0.5f, -0.5f, -0.5f},  {  0.0f, -1.0f,  0.0f }, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },	// 4 19

			// Top Face										
			Vertex{ {-0.5f,  0.5f,  0.5f},  {  0.0f,  1.0f,  0.0f }, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },	// 3 20
			Vertex{ { 0.5f,  0.5f,  0.5f},  {  0.0f,  1.0f,  0.0f }, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },	// 2 21
			Vertex{ { 0.5f,  0.5f, -0.5f},  {  0.0f,  1.0f,  0.0f }, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },	// 6 22
			Vertex{ {-0.5f,  0.5f, -0.5f},  {  0.0f,  1.0f,  0.0f }, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f} }	// 7 23
		};

		std::vector<uint32_t> indices =
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

		CalculateTandBTriangle(vertices, indices);

		return CreateRef<Mesh>(vertices, indices, Primitive::Cube);
	}

	
	Ref<Mesh> MeshFactory::Sphere(float radius)
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
				vertex.Normal = { cosPhi * sinTheta, cosTheta, sinPhi * sinTheta };
				vertex.TexCoord = { texS, texT };
				vertex.Position = { radius * vertex.Normal.x, radius * vertex.Normal.y, radius * vertex.Normal.z };
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

		CalculateTandBTriangle(vertices, indices);

		return CreateRef<Mesh>(vertices, indices, Primitive::Sphere);
	}

	static int AddIcosphereVertex(const glm::vec3& v, std::vector<Vertex>& vertices, uint32_t* index)
	{
		float length = glm::length(v);
		glm::vec3 unitSphereV = v / length;
		Vertex vert;
		vert.Position = unitSphereV;
		vert.Normal = unitSphereV;
		vert.TexCoord.x = atan2(unitSphereV.z, unitSphereV.x) / (2.0 * PI) + 0.5;
		vert.TexCoord.y = asin(unitSphereV.y) / PI + 0.5;
		vertices.push_back(vert);
		return (*(index))++;
	}

	static uint32_t GetIcosphereMidpoint(int p1, int p2, std::unordered_map<uint64_t, int>& midpointCache, std::vector<Vertex>& vertices, uint32_t* index)
	{
		bool firstIsSmaller = p1 < p2;
		uint64_t smallerIndex = firstIsSmaller ? p1 : p2;
		uint64_t largerIndex = firstIsSmaller ? p2 : p1;
		uint64_t key = (smallerIndex << 32) + largerIndex;

		if (midpointCache.find(key) != midpointCache.end())
			return midpointCache[key];

		Vertex v1 = vertices[p1];
		Vertex v2 = vertices[p2];
		glm::vec3 mid = (v1.Position + v2.Position) / 2.0f;
		int vertexIndex = AddIcosphereVertex(mid, vertices, index);
		midpointCache[key] = vertexIndex;
		return vertexIndex;
	}

	
	Ref<Mesh> MeshFactory::Icosphere(uint32_t level, float radius)
	{
		float t = (1.0f + glm::sqrt(5.0f)) / 2.0f;
		
		uint32_t vertexIndex = 0;
		std::vector<Vertex> vertices;

		AddIcosphereVertex({ -1.0f,  t, 0.0f }, vertices, &vertexIndex);
		AddIcosphereVertex({  1.0f,  t, 0.0f }, vertices, &vertexIndex);
		AddIcosphereVertex({ -1.0f, -t, 0.0f }, vertices, &vertexIndex);
		AddIcosphereVertex({  1.0f, -t, 0.0f }, vertices, &vertexIndex);

		AddIcosphereVertex({ -0.0f, -1.0f,  t }, vertices, &vertexIndex);
		AddIcosphereVertex({ -0.0f,  1.0f,  t }, vertices, &vertexIndex);
		AddIcosphereVertex({ -0.0f, -1.0f, -t }, vertices, &vertexIndex);
		AddIcosphereVertex({ -0.0f,  1.0f, -t }, vertices, &vertexIndex);

		AddIcosphereVertex({  t, 0.0f, -1.0f }, vertices, &vertexIndex);
		AddIcosphereVertex({  t, 0.0f,  1.0f }, vertices, &vertexIndex);
		AddIcosphereVertex({ -t, 0.0f, -1.0f }, vertices, &vertexIndex);
		AddIcosphereVertex({ -t, 0.0f,  1.0f }, vertices, &vertexIndex);

		struct Triangle { uint32_t A, B, C; };

		std::vector<Triangle> facesOne;

		facesOne.push_back({ 0, 11, 5 });
		facesOne.push_back({ 0, 5, 1 });
		facesOne.push_back({ 0, 1, 7 });
		facesOne.push_back({ 0, 7, 10 });
		facesOne.push_back({ 0, 10, 11 });

		// 5 adjacent faces
		facesOne.push_back({ 1, 5, 9 });
		facesOne.push_back({ 5, 11, 4 });
		facesOne.push_back({ 11, 10, 2 });
		facesOne.push_back({ 10, 7, 6 });
		facesOne.push_back({ 7, 1, 8 });;

		// 5 faces around point 3
		facesOne.push_back({ 3, 9, 4 });
		facesOne.push_back({ 3, 4, 2 });
		facesOne.push_back({ 3, 2, 6 });
		facesOne.push_back({ 3, 6, 8 });
		facesOne.push_back({ 3, 8, 9 });

		// 5 adjacent faces
		facesOne.push_back({ 4, 9, 5 });
		facesOne.push_back({ 2, 4, 11 });
		facesOne.push_back({ 6, 2, 10 });
		facesOne.push_back({ 8, 6, 7 });
		facesOne.push_back({ 9, 8, 1 });

		std::unordered_map<uint64_t, int> midpointCache;

		for (uint32_t i = 0; i < level; i++)
		{
			std::vector<Triangle> facesTwo;

			for (auto triangle : facesOne)
			{
				uint32_t a = GetIcosphereMidpoint(triangle.A, triangle.B, midpointCache, vertices, &vertexIndex);
				uint32_t b = GetIcosphereMidpoint(triangle.B, triangle.C, midpointCache, vertices, &vertexIndex);
				uint32_t c = GetIcosphereMidpoint(triangle.C, triangle.A, midpointCache, vertices, &vertexIndex);

				facesTwo.push_back({ triangle.A, a, c });
				facesTwo.push_back({ triangle.B, b, a });
				facesTwo.push_back({ triangle.C, c, b });
				facesTwo.push_back({ a, b, c });
			}

			facesOne = facesTwo;
		}

		std::vector<uint32_t> indices;
		for (auto triangle : facesOne)
		{
			indices.push_back(triangle.A);
			indices.push_back(triangle.B);
			indices.push_back(triangle.C);
		}

		CalculateTandBTriangle(vertices, indices);
		
		return CreateRef<Mesh>(vertices, indices, Primitive::Icosphere);
	}

	Ref<Mesh> MeshFactory::Skybox()
	{
		std::vector<uint32_t> Indices =
		{
			// Back Face
			0, 1, 2,
			2, 3, 0,

			// Left Face
			4, 1, 0,
			0, 5, 4,

			// Right Face
			2, 6, 7,
			7, 3, 2,

			// Top Face
			0, 3, 7,
			7, 5, 0,

			// Front Face
			4, 5, 7,
			7, 6, 4,

			// Bottom Face
			4, 6, 2,
			2, 1, 4
		};

		std::vector Vertices =
		{
			Vertex{ {-1.0f,  1.0f, -1.0f},  {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
			Vertex{ {-1.0f, -1.0f, -1.0f},  {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
			Vertex{ { 1.0f, -1.0f, -1.0f},  {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} }, 
			Vertex{ { 1.0f,  1.0f, -1.0f},  {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
			
			Vertex{ { -1.0f, -1.0f,  1.0f},  {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} }, 
			Vertex{ { -1.0f,  1.0f,  1.0f},  {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} }, 
			Vertex{ {  1.0f, -1.0f,  1.0f},  {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} }, 
			Vertex{ {  1.0f,  1.0f,  1.0f},  {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} }, 
		};

		return CreateRef<Mesh>(Vertices, Indices, Primitive::Skybox);
	}

	Primitive MeshFactory::StringToPrimitiveType(const std::string& Name)
	{
		if(Name == "Cube")				return Primitive::Cube;
		if(Name == "Sphere")			return Primitive::Sphere;
		if(Name == "Quad") 				return Primitive::Quad;
		if(Name == "Plane") 			return Primitive::Plane;
		if(Name == "Full Screen Quad") 	return Primitive::FullScreenQuad;
		if(Name == "TessellatedQuad") 	return Primitive::TessellatedQuad;
		if(Name == "Skybox") 			return Primitive::Skybox;
		return Primitive::None;
	}

	std::string MeshFactory::MeshPrimitiveToString(Primitive primitive)
	{
		switch(primitive)
		{
		case Primitive::None: return "None";
		case Primitive::Triangle: return "Triangle";
		case Primitive::Quad: return "Quad";
		case Primitive::FullScreenQuad: return "Full Screen Quad";
		case Primitive::Plane: return "Plane";
		case Primitive::Cube: return "Cube";
		case Primitive::Sphere: return "Sphere";
		case Primitive::TessellatedQuad: return "Tessellated Quad";
		case Primitive::Icosphere: return "Icosphere";
		case Primitive::Skybox: return "Skybox";
		default: ;
		}
		return {};
	}

	Ref<Mesh> MeshFactory::TessellatedQuad(uint32_t resolution)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		// Space between each vertex.  This is for a unit quad (-0.5f, -0.5f) - (0.5f, 0.5).
		float spacing = 1.0f / (float)resolution;
		
		glm::vec3 bottomLeft{ -0.5f, -0.5f, 0.0f };

		for (uint32_t y = 0; y < resolution; y++)
		{
			for (uint32_t x = 0; x < resolution; x++)
			{
				glm::vec3 position{ bottomLeft.x + (float)x * spacing, bottomLeft.y + (float)y * spacing, 0.0f };

				glm::vec2 texCoord{ (float)x / resolution, (float)y / resolution };
				glm::vec3 normal{ 0.0f, 0.0f, 1.0f };

				vertices.push_back({ position, normal, {}, {}, texCoord });

				if (x == resolution - 1 || y == resolution - 1) continue;

				uint32_t a = y * resolution + x;
				uint32_t b = y * resolution + x + resolution;
				uint32_t c = y * resolution + x + resolution + 1;
				indices.push_back(a);
				indices.push_back(b);
				indices.push_back(c);

				uint32_t d = a;
				uint32_t e = y * resolution + x + resolution + 1;
				uint32_t f = a + 1;
				indices.push_back(d);
				indices.push_back(e);
				indices.push_back(f);
			}
		}

		return CreateRef<Mesh>(vertices, indices, Primitive::TessellatedQuad);
	}


}