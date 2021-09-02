#include "ohmpch.h"
#include "Ohm/Rendering/UniformBuffer.h"
#include <glad/glad.h>

namespace Ohm
{
	UniformBuffer::UniformBuffer(uint32_t size, uint32_t binding)
	{
		glCreateBuffers(1, &m_ID);
		glNamedBufferData(m_ID, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_ID);
	}

	UniformBuffer::~UniformBuffer()
	{
		glDeleteBuffers(1, &m_ID);
	}

	void UniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		glNamedBufferSubData(m_ID, offset, size, data);
	}
}