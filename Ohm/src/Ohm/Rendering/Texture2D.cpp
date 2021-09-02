#include "ohmpch.h"
#include "Ohm/Rendering/Texture2D.h"

#include <glad/glad.h>
#include <stb_image.h>

namespace Ohm
{
	Texture2D::Texture2D(const std::string& filePath)
	{
		int width, height;
		int channels;

		stbi_set_flip_vertically_on_load(1);

		stbi_uc* data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);

		m_Width = width;
		m_Height = height;

		m_InternalFormat = channels == 4 ? GL_RGBA8 : GL_RGB8;
		m_DataFormat = channels == 4 ? GL_RGBA : GL_RGB;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
		glTextureStorage2D(m_ID, 1, m_InternalFormat, width, height);

		glTextureParameteri(m_ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_ID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureParameteri(m_ID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_ID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureSubImage2D(m_ID, 0, 0, 0, width, height, m_DataFormat, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
	}

	Texture2D::Texture2D(uint32_t width, uint32_t height)
	{
		m_Width = width;
		m_Height = height;

		m_InternalFormat = GL_RGBA8;
		m_DataFormat = GL_RGBA;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
		glTextureStorage2D(m_ID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_ID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureParameteri(m_ID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_ID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	Texture2D::~Texture2D()
	{
		glDeleteTextures(1, &m_ID);
	}

	void Texture2D::Bind(uint32_t unit) const
	{
		glBindTextureUnit(unit, m_ID);
	}

	void Texture2D::SetData(const void* data, uint32_t size)
	{
		uint32_t bytesPerPixel = m_DataFormat == GL_RGBA ? 4 : 3;
		if (size != m_Width * m_Height * bytesPerPixel)
		{
			OHM_ERROR("Data must fill entire texture!");
		}
		else
		{
			glTextureSubImage2D(m_ID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
		}
	}
}