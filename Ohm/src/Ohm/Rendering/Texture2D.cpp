#include "ohmpch.h"
#include "Ohm/Rendering/Texture2D.h"

#include <glad/glad.h>
#include <stb_image.h>

namespace Ohm
{
	Texture2D::Texture2D(const std::string& filePath, const Texture2DSpecification& specification)
		:m_Specification(specification)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
		glBindTexture(GL_TEXTURE_2D, m_ID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, TextureUtils::ConvertWrapMode(specification.SamplerWrap));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, TextureUtils::ConvertWrapMode(specification.SamplerWrap));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TextureUtils::ConvertMinMagFilterMode(specification.MinFilter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TextureUtils::ConvertMinMagFilterMode(specification.MinFilter));

		stbi_set_flip_vertically_on_load(1);

		int channels, width, height;
		unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);

		m_Specification.Width = width;
		m_Specification.Height = height;

		if (m_Specification.InternalFormat == TextureUtils::ImageInternalFormat::FromImage && m_Specification.DataLayout == TextureUtils::ImageDataLayout::FromImage)
		{
			switch (channels)
			{
			case 1:
			{
				m_Specification.InternalFormat = TextureUtils::ImageInternalFormat::R8;
				m_Specification.DataLayout = TextureUtils::ImageDataLayout::Red;
				break;
			}
			case 2:
			{
				m_Specification.InternalFormat = TextureUtils::ImageInternalFormat::RG8;
				m_Specification.DataLayout = TextureUtils::ImageDataLayout::RG;
				break;
			}
			case 3:
			{
				m_Specification.InternalFormat = TextureUtils::ImageInternalFormat::RGB8;
				m_Specification.DataLayout = TextureUtils::ImageDataLayout::RGB;
				break;
			}
			case 4:
			{
				m_Specification.InternalFormat = TextureUtils::ImageInternalFormat::RGBA8;
				m_Specification.DataLayout = TextureUtils::ImageDataLayout::RGBA;
				break;
			}
			}
		}

		if (data)
		{
			GLenum internalFormat = TextureUtils::ConvertInternalFormatMode(m_Specification.InternalFormat);
			GLenum dataFormat = TextureUtils::ConverDataLayoutMode(m_Specification.DataLayout);
			GLenum dataType = TextureUtils::ConvertImageDataType(m_Specification.DataType);
			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Specification.Width, m_Specification.Height, 0, dataFormat, dataType, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			stbi_image_free(data);
		}
		else
		{
			OHM_ERROR("Failed to load texture.");
		}
	}

	Texture2D::Texture2D(const Texture2DSpecification& specification)
		:m_Specification(specification)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
		glBindTexture(GL_TEXTURE_2D, m_ID);

		GLenum wrapS = TextureUtils::ConvertWrapMode(specification.SamplerWrap);
		GLenum wrapT = TextureUtils::ConvertWrapMode(specification.SamplerWrap);
		GLenum minFilter = TextureUtils::ConvertMinMagFilterMode(specification.MinFilter);
		GLenum magFilter = TextureUtils::ConvertMinMagFilterMode(specification.MagFilter);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

		uint32_t mips = GetMipCount();
		glTextureStorage2D(m_ID, mips, TextureUtils::ConvertInternalFormatMode(specification.InternalFormat), specification.Width, specification.Height);
	}

	Texture2D::~Texture2D()
	{
		glDeleteTextures(1, &m_ID);
	}

	void Texture2D::BindToSamplerSlot(uint32_t unit) const
	{
		glBindTextureUnit(unit, m_ID);
	}

	void Texture2D::UnbindFromSamplerSlot(uint32_t unit) const
	{
		glBindTextureUnit(unit, 0);
	}

	void Texture2D::Invalidate()
	{
		if (m_ID)
			glDeleteTextures(1, &m_ID);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
		glBindTexture(GL_TEXTURE_2D, m_ID);

		GLenum wrapS = TextureUtils::ConvertWrapMode(m_Specification.SamplerWrap);
		GLenum wrapT = TextureUtils::ConvertWrapMode(m_Specification.SamplerWrap);
		GLenum minFilter = TextureUtils::ConvertMinMagFilterMode(m_Specification.MinFilter);
		GLenum magFilter = TextureUtils::ConvertMinMagFilterMode(m_Specification.MagFilter);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

		uint32_t mips = GetMipCount();
		glTextureStorage2D(m_ID, mips, TextureUtils::ConvertInternalFormatMode(m_Specification.InternalFormat), m_Specification.Width, m_Specification.Height);
	}

	void Texture2D::BindToImageSlot(uint32_t unit, uint32_t level, TextureUtils::TextureAccessLevel access, TextureUtils::TextureShaderDataFormat shaderDataFormat)
	{
		GLenum glShaderDataFormat = TextureUtils::ConvertShaderFormatType(shaderDataFormat);
		GLenum internalFormat = TextureUtils::ConvertInternalFormatMode(m_Specification.InternalFormat);

		if (glShaderDataFormat != internalFormat)
		{
			std::cout << "Shader Data Format and Internal format must match!" << "\n";
			return;
		}

		glBindImageTexture(unit, m_ID, level, GL_FALSE, 0, TextureUtils::ConvertTextureAccessLevel(access), TextureUtils::ConvertShaderFormatType(shaderDataFormat));
	}

	void Texture2D::Resize(uint32_t width, uint32_t height)
	{
		m_Specification.Width = width;
		m_Specification.Height = height;
		Invalidate();
	}

	std::pair<uint32_t, uint32_t> Texture2D::GetMipDimensions(uint32_t mip) const
	{
		uint32_t width = m_Specification.Width;
		uint32_t height = m_Specification.Height;

		while (mip != 0)
		{
			width /= 2;
			height /= 2;
			mip--;
		}

		return { width, height };
	}

	void Texture2D::SetData(const void* data, uint32_t size)
	{
		uint32_t bytesPerPixel = m_Specification.DataLayout == TextureUtils::ImageDataLayout::RGBA ? 4 : 3;
		if (size != m_Specification.Width * m_Specification.Height * bytesPerPixel)
		{
			OHM_ERROR("Data must fill entire texture!");
		}
		else
		{
			GLenum pixelLayout = TextureUtils::ConverDataLayoutMode(m_Specification.DataLayout);
			GLenum type = TextureUtils::ConvertImageDataType(m_Specification.DataType);
			glTextureSubImage2D(m_ID, 0, 0, 0, m_Specification.Width, m_Specification.Height, pixelLayout, type, data);
		}
	}
}