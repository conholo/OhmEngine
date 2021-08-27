#pragma once

#include "Ohm/Rendering/BufferLayout.h"

namespace Ohm
{
	class VertexBuffer
	{
	public:
		VertexBuffer(float* vertices, uint32_t size);
		~VertexBuffer();

		void Bind() const;
		void Unbind() const;

		void SetLayout(const BufferLayout& layout) { m_Layout = layout; }
		const BufferLayout& GetLayout() const { return m_Layout; }

	private:
		uint32_t m_ID;
		BufferLayout m_Layout;
	};
}