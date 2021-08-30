#pragma once

#include "Ohm/Rendering/Vertex.h"
#include "Ohm/Rendering/BufferLayout.h"
#include "Ohm/Rendering/IndexBuffer.h"
#include "Ohm/Rendering/VertexBuffer.h"

namespace Ohm
{
	enum class Primitive
	{
		Quad, Cube
	};


	class Mesh
	{
	public:

		Mesh() = default;
		Mesh(const Mesh&) = default;
		Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices);

		static Ref<Mesh> CreatePrimitive(Primitive primitive);

		const Ref<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }
		const Ref<VertexBuffer>& GetVertexBuffer() const { return m_VertexBuffer; }

	private:
		void CreateRenderPrimitives(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
		static Ref<Mesh> Quad();
		static Ref<Mesh> Cube();

	private:
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;
	};


}