#pragma once

#include "Ohm/Rendering/Vertex.h"
#include "Ohm/Rendering/BufferLayout.h"
#include "Ohm/Rendering/IndexBuffer.h"
#include "Ohm/Rendering/VertexBuffer.h"

namespace Ohm
{
	enum class Primitive
	{
		None = 0, Quad, Cube, Sphere, Plane, FullScreenQuad, UnitCube
	};

	class Mesh
	{
	public:
		Mesh() = default;
		Mesh(const Mesh&) = default;
		Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, Primitive primitive);

		const Ref<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }
		const Ref<VertexBuffer>& GetVertexBuffer() const { return m_VertexBuffer; }

		const std::vector<Vertex>& GetVertices() const { return m_Vertices; }
		const std::vector<uint32_t>& GetIndices() const { return m_Indices; }
		const Primitive GetPrimitiveType() const { return m_PrimitiveType; }

		void Unbind() const;
		void Bind() const;

		static Ref<Mesh> CreateUnitCube();
		static Ref<Mesh> CreatePrimitive(Primitive primitive);
		static Ref<Mesh> Sphere(float radius);
		static std::string PrimitiveToString(Primitive primitive);

	private:
		void CreateRenderPrimitives(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
		static Ref<Mesh> Quad();
		static Ref<Mesh> FullScreenQuad();
		static Ref<Mesh> Cube();
		static Ref<Mesh> Plane();

	private:
		Primitive m_PrimitiveType;
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;
	};

	inline std::ostream& operator <<(std::ostream& stream, Primitive primitive)
	{
		return stream << Mesh::PrimitiveToString(primitive);
	}
}