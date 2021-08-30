#pragma once

#include "Ohm/Rendering/Vertex.h"

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
		Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
			:m_Vertices(vertices), m_Indices(indices){ }
		Mesh(const std::initializer_list<Vertex>& vertices, const std::initializer_list<uint32_t>& indices)
			:m_Vertices(vertices), m_Indices(indices) { }

		std::vector<Vertex>::iterator begin() { return m_Vertices.begin(); }
		std::vector<Vertex>::iterator end() { return m_Vertices.end(); }

		std::vector<Vertex>::const_iterator begin() const { return m_Vertices.begin(); }
		std::vector<Vertex>::const_iterator end() const { return m_Vertices.end(); }

		std::vector<Vertex>& GetVertices() { return m_Vertices; }
		std::vector<uint32_t>& GetIndices() { return  m_Indices; }


		static Ref<Mesh> CreatePrimitive(Primitive primitive);

	private:
		static Ref<Mesh> Quad();
		static Ref<Mesh> Cube();

	private:
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
	};


}