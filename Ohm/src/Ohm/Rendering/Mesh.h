#pragma once

#include "Ohm/Rendering/VertexArray.h"
#include "Ohm/Rendering/Vertex.h"
#include "Ohm/Rendering/BufferLayout.h"
#include "Ohm/Rendering/IndexBuffer.h"
#include "Ohm/Rendering/VertexBuffer.h"

namespace Ohm
{
	enum class Primitive { None = 0, Triangle, Quad, FullScreenQuad, Plane, Cube, Sphere, TessellatedQuad, Icosphere, Skybox };

	class Mesh
	{
	public:
		Mesh() = default;
		Mesh(const Mesh&) = default;
		Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, Primitive primitive = Primitive::None);

		const Ref<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }
		const Ref<VertexBuffer>& GetVertexBuffer() const { return m_VertexBuffer; }

		const std::vector<Vertex>& GetVertices() const { return m_Vertices; }
		const std::vector<uint32_t>& GetIndices() const { return m_Indices; }
		Primitive GetPrimitiveType() const { return m_PrimitiveType; }

		void Unbind() const;
		void Bind() const;
		const Ref<VertexArray>& GetVAO() { return m_VertexArray; }

	private:
		void CreateRenderPrimitives();

		Primitive m_PrimitiveType;
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;

		Ref<VertexArray> m_VertexArray;
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;
	};

	class MeshFactory
	{
	public:
		static Ref<Mesh> Create(Primitive primitiveType);
		static Ref<Mesh> Plane();
		static Ref<Mesh> Triangle();
		static Ref<Mesh> Quad();
		static Ref<Mesh> FullScreenQuad();
		static Ref<Mesh> Cube();
		static Ref<Mesh> TessellatedQuad(uint32_t resolution);
		static Ref<Mesh> Sphere(float radius);
		static Ref<Mesh> Icosphere(uint32_t level, float radius);
		static Ref<Mesh> Skybox();

		static Primitive StringToPrimitiveType(const std::string& Name);
		static std::string MeshPrimitiveToString(Primitive primitive);
	};
}