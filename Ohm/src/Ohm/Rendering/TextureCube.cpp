#include "ohmpch.h"
#include "Ohm/Rendering/TextureCube.h"

#include <glad/glad.h>
#include <stb_image.h>

namespace Ohm
{

	TextureCube::TextureCube(const TextureCubeSpecification& specification, const std::vector<std::string>& cubeFaceFiles)
		:m_Specification(specification), m_Name(specification.Name)
	{
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_ID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);

		int Width, Height, Channels;

		for (uint32_t i = 0; i < cubeFaceFiles.size(); i++)
		{
			if (unsigned char* Data = stbi_load(cubeFaceFiles[i].c_str(), &Width, &Height, &Channels, 0))
			{
				m_Specification.InternalFormat = Channels == 4 ? TextureUtils::ImageInternalFormat::RGBA8 : TextureUtils::ImageInternalFormat::RGB8;
				m_Specification.DataLayout = Channels == 4 ? TextureUtils::ImageDataLayout::RGBA : TextureUtils::ImageDataLayout::RGB;

				const GLenum InternalFormat = ConvertInternalFormatMode(m_Specification.InternalFormat);
				const GLenum DataFormat = ConverDataLayoutMode(m_Specification.DataLayout);
				const GLenum DataType = ConvertImageDataType(m_Specification.DataType);
				m_Specification.Dimension = Height;

				const auto MipCount = GetMipLevelCount();
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, MipCount, InternalFormat, Width, Height, 0, DataFormat, DataType, Data);
				stbi_image_free(Data);
			}
			else
			{
				OHM_ERROR("Failed to load face: {} for Cube Map Texture from file '{}'.", i, cubeFaceFiles[i]);
				stbi_image_free(Data);
			}
		}
		const GLenum WrapModeS = ConvertWrapMode(m_Specification.SamplerWrapS);
		const GLenum WrapModeT = ConvertWrapMode(m_Specification.SamplerWrapT);
		const GLenum WrapModeR = ConvertWrapMode(m_Specification.SamplerWrapR);
		const GLenum MinFilter = ConvertMinMagFilterMode(m_Specification.MinFilter);
		const GLenum MagFilter = ConvertMinMagFilterMode(m_Specification.MagFilter);

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, WrapModeS);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, WrapModeT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, WrapModeR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, MinFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, MagFilter);
	}

	/**
	 * \brief A Cubemap.
	 * \param Specification The Cubemap Specification.
	 */
	TextureCube::TextureCube(const TextureCubeSpecification& Specification)
		:m_Specification(Specification), m_ID(0), m_Name(Specification.Name)
	{
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_ID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);

		const GLenum WrapModeS = ConvertWrapMode(m_Specification.SamplerWrapS);
		const GLenum WrapModeT = ConvertWrapMode(m_Specification.SamplerWrapT);
		const GLenum WrapModeR = ConvertWrapMode(m_Specification.SamplerWrapR);
		const GLenum MinFilter = ConvertMinMagFilterMode(m_Specification.MinFilter);
		const GLenum MagFilter = ConvertMinMagFilterMode(m_Specification.MagFilter);
		const GLenum InternalFormat = ConvertInternalFormatMode(m_Specification.InternalFormat);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, WrapModeS);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, WrapModeT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, WrapModeR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, MinFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, MagFilter);
		const auto MipCount = GetMipLevelCount();
		glTexStorage2D(GL_TEXTURE_CUBE_MAP, MipCount, InternalFormat, Specification.Dimension, Specification.Dimension);
	}

	void TextureCube::Invalidate(const TextureCubeSpecification& Specification)
	{
		OHM_TRACE("Invalidating Cubemap: {}", m_Specification.Name);
		if(Specification.Name != m_Specification.Name)
			OHM_TRACE("\t New name for Invalidated Cubemap: {}", m_Specification.Name);
		m_Specification = Specification;
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_ID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);

		const GLenum WrapModeS = ConvertWrapMode(m_Specification.SamplerWrapS);
		const GLenum WrapModeT = ConvertWrapMode(m_Specification.SamplerWrapT);
		const GLenum WrapModeR = ConvertWrapMode(m_Specification.SamplerWrapR);
		const GLenum MinFilter = ConvertMinMagFilterMode(m_Specification.MinFilter);
		const GLenum MagFilter = ConvertMinMagFilterMode(m_Specification.MagFilter);
		const GLenum InternalFormat = ConvertInternalFormatMode(m_Specification.InternalFormat);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, WrapModeS);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, WrapModeT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, WrapModeR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, MinFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, MagFilter);
		const auto MipCount = GetMipLevelCount();
		glTexStorage2D(GL_TEXTURE_CUBE_MAP, MipCount, InternalFormat, Specification.Dimension, Specification.Dimension);
	}

	TextureCube::~TextureCube()
	{
		glDeleteTextures(1, &m_ID);
	}

	void TextureCube::BindToSamplerSlot(uint32_t slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTextureUnit(slot, m_ID);
	}

	void TextureCube::Unbind()
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	void TextureCube::BindToImageSlot(uint32_t Binding, uint32_t MipLevel, TextureUtils::TextureAccessLevel AccessLevel, TextureUtils::TextureShaderDataFormat ShaderDataFormat) const
	{
		const GLenum GLShaderDataFormat = ConvertShaderFormatType(ShaderDataFormat);
		const GLenum InternalFormat = ConvertInternalFormatMode(m_Specification.InternalFormat);

		if (GLShaderDataFormat != InternalFormat)
		{
			OHM_ERROR("Failure Binding '{}' TextureCube to Image Slot: Shader Data Format and Internal format must match!", m_Specification.Name);
			return;
		}

		glBindImageTexture(Binding, m_ID, MipLevel, GL_TRUE, 0, ConvertTextureAccessLevel(AccessLevel), ConvertShaderFormatType(ShaderDataFormat));
	}

	void TextureCube::SetData(const void* data, size_t size) const
	{
		const uint32_t bytesPerPixel = m_Specification.DataLayout == TextureUtils::ImageDataLayout::RGBA ? 4 : 3;
		ASSERT(size == bytesPerPixel * m_Specification.Dimension * m_Specification.Dimension, "Data size must match entire face of texture.");

		const GLenum PixelLayout = ConverDataLayoutMode(m_Specification.DataLayout);
		const GLenum DataType = ConvertImageDataType(m_Specification.DataType);

		const std::vector Faces =
		{
			GL_TEXTURE_CUBE_MAP_POSITIVE_X,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		};

		for (const auto Face : Faces)
			glTexSubImage2D(Face, 0, 0, 0, m_Specification.Dimension, m_Specification.Dimension, PixelLayout, DataType, data);
	}

	std::pair<glm::uint32_t, glm::uint32_t> TextureCube::GetMipSize(uint32_t Mip) const
	{
		uint32_t Width = m_Specification.Dimension;
		uint32_t Height = m_Specification.Dimension;
		while (Mip != 0)
		{
			Width /= 2;
			Height /= 2;
			Mip--;
		}

		return { Width, Height };
	}
	
	uint32_t TextureCube::GetMipLevelCount() const
	{
		return TextureUtils::CalculateMipLevelCount(m_Specification.Dimension, m_Specification.Dimension);
	}
}