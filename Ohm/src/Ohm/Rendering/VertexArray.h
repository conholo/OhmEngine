#pragma once

#include "Ohm/Rendering/VertexBuffer.h"
#include "Ohm/Rendering/IndexBuffer.h"

namespace Ohm
{
	class VertexArray
	{
	public:
		VertexArray();
		~VertexArray();

		void Bind() const;
		void Unbind() const;

		void EnableVertexAttributes(const Ref<VertexBuffer>& vertexBuffer);

	private:
		uint32_t m_ID;
	};
}