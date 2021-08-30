#include "ohmpch.h"
#include "Ohm/Rendering/VertexArray.h"

#include <glad/glad.h>

namespace Ohm
{
	GLenum GLEnumFromShaderDataType(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Mat3:
			case ShaderDataType::Mat4:
			case ShaderDataType::Float:		
			case ShaderDataType::Float2:
			case ShaderDataType::Float3:	return GL_FLOAT;
			case ShaderDataType::Int:		return GL_INT;
			default:						return GL_FLOAT;
		}
	}

	VertexArray::VertexArray()
	{
		glCreateVertexArrays(1, &m_ID);
		glBindVertexArray(m_ID);
	}

	VertexArray::~VertexArray()
	{
		glDeleteVertexArrays(1, &m_ID);
	}

	void VertexArray::Bind() const
	{
		glBindVertexArray(m_ID);
	}

	void VertexArray::Unbind() const
	{
		glBindVertexArray(0);
	}

	void VertexArray::EnableVertexAttributes(const Ref<VertexBuffer>& vertexBuffer)
	{
		glBindVertexArray(m_ID);
		vertexBuffer->Bind();

		const auto& layout = vertexBuffer->GetLayout();

		uint32_t index = 0;
		for (const auto& element : layout)
		{
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(
				index,
				element.GetComponentCount(),
				GLEnumFromShaderDataType(element.Type),
				element.Normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)element.Offset
			);
			index++;
		}
	}
}