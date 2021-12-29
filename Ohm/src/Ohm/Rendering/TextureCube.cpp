#include "ohmpch.h"
#include "Ohm/Rendering/TextureCube.h"

#include <glad/glad.h>
#include <stb_image.h>

namespace Ohm
{

	TextureCube::TextureCube(const TextureCubeSpecification& spec)
		:m_Specification(spec)
	{
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_ID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);

		GLenum wrapMode = TextureUtils::ConvertWrapMode(m_Specification.SamplerWrap);
		GLenum minFilter = TextureUtils::ConvertMinMagFilterMode(m_Specification.MinFilter);
		GLenum magFilter = TextureUtils::ConvertMinMagFilterMode(m_Specification.MagFilter);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrapMode);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrapMode);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrapMode);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, magFilter);

		uint32_t mips = GetMipCount();
		glTexStorage2D(GL_TEXTURE_CUBE_MAP, mips, TextureUtils::ConvertInternalFormatMode(spec.InternalFormat), spec.Width, spec.Height);
	}

	TextureCube::TextureCube(const TextureCubeSpecification& spec, const std::vector<std::string>& cubeFaceFiles)
		:m_Specification(spec)
	{
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_ID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);

		int width, height, channels;

		for (uint32_t i = 0; i < cubeFaceFiles.size(); i++)
		{
			unsigned char* data = stbi_load(cubeFaceFiles[i].c_str(), &width, &height, &channels, 0);

			if (data)
			{
				m_Specification.InternalFormat = channels == 4 ? TextureUtils::ImageInternalFormat::RGBA8 : TextureUtils::ImageInternalFormat::RGB8;
				m_Specification.DataLayout = channels == 4 ? TextureUtils::ImageDataLayout::RGBA : TextureUtils::ImageDataLayout::RGB;

				GLenum internalFormat = TextureUtils::ConvertInternalFormatMode(m_Specification.InternalFormat);
				GLenum dataFormat = TextureUtils::ConverDataLayoutMode(m_Specification.DataLayout);
				GLenum dataType = TextureUtils::ConvertImageDataType(m_Specification.DataType);
				m_Specification.Width = width;
				m_Specification.Height = height;

				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, dataFormat, dataType, data);
				glGenerateMipmap(GL_TEXTURE_2D);
				stbi_image_free(data);
			}
			else
			{
				OHM_ERROR("Failed to load face: {} for Texture Cube.  Aborting process.", i);
				stbi_image_free(data);
			}
		}

		GLenum wrapMode = TextureUtils::ConvertWrapMode(m_Specification.SamplerWrap);
		GLenum minFilter = TextureUtils::ConvertMinMagFilterMode(m_Specification.MinFilter);
		GLenum magFilter = TextureUtils::ConvertMinMagFilterMode(m_Specification.MagFilter);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrapMode);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrapMode);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrapMode);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, magFilter);
	}

	TextureCube::~TextureCube()
	{
		glDeleteTextures(1, &m_ID);
	}

	void TextureCube::BindToSamplerSlot(uint32_t unit)
	{
		glBindTextureUnit(unit, m_ID);
	}

	void TextureCube::UnbindFromSamplerSlot(uint32_t unit)
	{
		glBindTextureUnit(unit, 0);
	}

	void TextureCube::Invalidate()
	{
		if (m_ID)
			glDeleteTextures(1, &m_ID);

		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_ID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);

		GLenum wrapMode = TextureUtils::ConvertWrapMode(m_Specification.SamplerWrap);
		GLenum minFilter = TextureUtils::ConvertMinMagFilterMode(m_Specification.MinFilter);
		GLenum magFilter = TextureUtils::ConvertMinMagFilterMode(m_Specification.MagFilter);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrapMode);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrapMode);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrapMode);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, magFilter);

		uint32_t mips = GetMipCount();
		glTexStorage2D(GL_TEXTURE_CUBE_MAP, mips, TextureUtils::ConvertInternalFormatMode(m_Specification.InternalFormat), m_Specification.Width, m_Specification.Height);
	}

	void TextureCube::BindToImageSlot(uint32_t unit, uint32_t level, TextureUtils::TextureAccessLevel access, TextureUtils::TextureShaderDataFormat shaderDataFormat, bool layered, uint32_t layer)
	{
		GLenum glShaderDataFormat = TextureUtils::ConvertShaderFormatType(shaderDataFormat);
		GLenum internalFormat = TextureUtils::ConvertInternalFormatMode(m_Specification.InternalFormat);

		if (glShaderDataFormat != internalFormat)
		{
			std::cout << "Shader Data Format and Internal format must match!" << "\n";
			return;
		}

		glBindImageTexture(unit, m_ID, level, layered ? GL_TRUE : GL_FALSE, layer, TextureUtils::ConvertTextureAccessLevel(access), TextureUtils::ConvertShaderFormatType(shaderDataFormat));
	}

	void TextureCube::Resize(uint32_t width, uint32_t height)
	{
		m_Specification.Width = width;
		m_Specification.Height = height;
		Invalidate();
	}

	void TextureCube::SetData(void* data, uint32_t size)
	{
		GLenum pixelLayout = TextureUtils::ConverDataLayoutMode(m_Specification.DataLayout);
		GLenum type = TextureUtils::ConvertImageDataType(m_Specification.DataType);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);
		glTexSubImage3D(GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0, m_Specification.Width, m_Specification.Height, 1, pixelLayout, type, data);
	}

	void TextureCube::EnableShaderAccessBarrierBit()
	{
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	std::pair<glm::uint32_t, glm::uint32_t> TextureCube::GetMipDimensions(uint32_t mip) const
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

}