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

		void ClearIndexBuffer() { m_IndexBuffer = nullptr; }
		const Ref<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }
		void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer);
		void EnableVertexAttributes(const Ref<VertexBuffer>& vertexBuffer);
		
	private:
		Ref<IndexBuffer> m_IndexBuffer;
		uint32_t m_ID;
	};
}