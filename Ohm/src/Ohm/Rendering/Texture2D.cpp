#include "ohmpch.h"
#include "Ohm/Rendering/Texture2D.h"

#include <glad/glad.h>
#include <stb_image.h>

namespace Ohm
{
	Texture2D::Texture2D(const Texture2DSpecification& specification)
		:m_Specification(specification), m_Name(specification.Name)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
		glBindTexture(GL_TEXTURE_2D, m_ID);

		GLenum wrapS = ConvertWrapMode(specification.WrapModeS);
		GLenum wrapT = ConvertWrapMode(specification.WrapModeT);
		GLenum minFilter = ConvertMinMagFilterMode(specification.MinFilterMode);
		GLenum magFilter = ConvertMinMagFilterMode(specification.MagFilterMode);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

		// Immutable-format Texture
		// Contents of the image can be modified, but it's storage requirements may not change.
		// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexStorage2D.xhtml

		uint32_t mips = GetMipLevelCount();
		glTextureStorage2D(m_ID, mips, ConvertInternalFormatMode(specification.InternalFormat), specification.Width, specification.Height);
	}

	Texture2D::Texture2D(const Texture2DSpecification& specification, void* data)
		:m_Specification(specification), m_Name(specification.Name)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
		glBindTexture(GL_TEXTURE_2D, m_ID);

		const GLenum wrapS = ConvertWrapMode(specification.WrapModeS);
		const GLenum wrapT = ConvertWrapMode(specification.WrapModeT);
		const GLenum minFilter = ConvertMinMagFilterMode(specification.MinFilterMode);
		const GLenum magFilter = ConvertMinMagFilterMode(specification.MagFilterMode);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

		const GLenum internalFormat = ConvertInternalFormatMode(m_Specification.InternalFormat);
		const GLenum dataFormat = ConverDataLayoutMode(m_Specification.PixelLayoutFormat);
		const GLenum dataType = ConvertImageDataType(m_Specification.DataType);

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Specification.Width, m_Specification.Height, 0, dataFormat, dataType, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	Texture2D::Texture2D(const std::string& filePath, const Texture2DSpecification& specification)
		:m_Specification(specification), m_FilePath(filePath)
	{
		if(specification.Name == "Texture2D")
		{
			size_t pos = m_FilePath.find_last_of("/") + 1;
			size_t size = m_FilePath.size() - pos;
			m_Name = m_FilePath.substr(pos, size);
		}
		else
		{
			m_Name = specification.Name;
		}

		glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
		glBindTexture(GL_TEXTURE_2D, m_ID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ConvertWrapMode(specification.WrapModeS));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ConvertWrapMode(specification.WrapModeT));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ConvertMinMagFilterMode(specification.MinFilterMode));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ConvertMinMagFilterMode(specification.MagFilterMode));

		stbi_set_flip_vertically_on_load(1);

		int channels, width, height;
		unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);

		m_Specification.Width = width;
		m_Specification.Height = height;

		if (m_Specification.InternalFormat == TextureUtils::ImageInternalFormat::FromImage && m_Specification.PixelLayoutFormat == TextureUtils::ImageDataLayout::FromImage)
		{
			switch (channels)
			{
				case 1:
				{
					m_Specification.InternalFormat = TextureUtils::ImageInternalFormat::R8;
					m_Specification.PixelLayoutFormat = TextureUtils::ImageDataLayout::Red;
					break;
				}
				case 2:
				{
					m_Specification.InternalFormat = TextureUtils::ImageInternalFormat::RG8;
					m_Specification.PixelLayoutFormat = TextureUtils::ImageDataLayout::RG;
					break;
				}
				case 3:
				{
					m_Specification.InternalFormat = TextureUtils::ImageInternalFormat::RGB8;
					m_Specification.PixelLayoutFormat = TextureUtils::ImageDataLayout::RGB;
					break;
				}
				case 4:
				{
					m_Specification.InternalFormat = TextureUtils::ImageInternalFormat::RGBA8;
					m_Specification.PixelLayoutFormat = TextureUtils::ImageDataLayout::RGBA;
					break;
				}
			}
		}

		if (data)
		{
			GLenum internalFormat = ConvertInternalFormatMode(m_Specification.InternalFormat);
			GLenum dataFormat = ConverDataLayoutMode(m_Specification.PixelLayoutFormat);
			GLenum dataType = ConvertImageDataType(m_Specification.DataType);
			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Specification.Width, m_Specification.Height, 0, dataFormat, dataType, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load texture. " << m_Specification.Name << "\n";
		}
	}

	Texture2D::~Texture2D()
	{
		glDeleteTextures(1, &m_ID);
	}

	void Texture2D::Invalidate()
	{
		if (m_ID)
			glDeleteTextures(1, &m_ID);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
		glBindTexture(GL_TEXTURE_2D, m_ID);

		GLenum wrapS = ConvertWrapMode(m_Specification.WrapModeS);
		GLenum wrapT = ConvertWrapMode(m_Specification.WrapModeT);
		GLenum minFilter = ConvertMinMagFilterMode(m_Specification.MinFilterMode);
		GLenum magFilter = ConvertMinMagFilterMode(m_Specification.MagFilterMode);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

		// Immutable-format Texture
		// Contents of the image can be modified, but it's storage requirements may not change.
		// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexStorage2D.xhtml

		uint32_t mips = GetMipLevelCount();
		glTextureStorage2D(m_ID, mips, ConvertInternalFormatMode(m_Specification.InternalFormat), m_Specification.Width, m_Specification.Height);
	}

	void Texture2D::Clear() const
	{
		glClearTexImage(m_ID, 0, ConverDataLayoutMode(m_Specification.PixelLayoutFormat), TextureUtils::ConvertImageDataType(m_Specification.DataType), nullptr);
	}

	void Texture2D::Resize(uint32_t width, uint32_t height)
	{
		m_Specification.Width = width;
		m_Specification.Height = height;
		Invalidate();
	}

	std::pair<uint32_t, uint32_t> Texture2D::GetMipSize(uint32_t mip) const
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

	uint32_t Texture2D::GetMipLevelCount() const
	{
		return TextureUtils::CalculateMipLevelCount(m_Specification.Width, m_Specification.Height);
	}

	void Texture2D::BindTextureIDToSamplerSlot(uint32_t slot, uint32_t id)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTextureUnit(slot, id);
	}

	void Texture2D::BindToSamplerSlot(uint32_t slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTextureUnit(slot, m_ID);
	}

	void Texture2D::Unbind(uint32_t slot) const
	{
		glBindTexture(GL_TEXTURE_2D, 0);

		if(slot != UINT32_MAX)
		{
			glActiveTexture(GL_TEXTURE0 + slot);
			glBindTextureUnit(slot, m_ID);
		}
	}

	void Texture2D::BindToImageSlot(uint32_t unit, uint32_t level, TextureUtils::TextureAccessLevel access, TextureUtils::TextureShaderDataFormat shaderDataFormat)
	{
		GLenum glShaderDataFormat = ConvertShaderFormatType(shaderDataFormat);
		GLenum internalFormat = ConvertInternalFormatMode(m_Specification.InternalFormat);

		if (glShaderDataFormat != internalFormat)
		{
			OHM_ERROR("Failure Binding '{}' Texture2D to Image Slot: Shader Data Format and Internal format must match!", m_Specification.Name);
			return;
		}

		glBindImageTexture(unit, m_ID, level, GL_FALSE, 0, ConvertTextureAccessLevel(access), ConvertShaderFormatType(shaderDataFormat));
	}

	void Texture2D::SetData(void* data, uint32_t size) const
	{
		uint32_t bytesPerPixel = m_Specification.PixelLayoutFormat == TextureUtils::ImageDataLayout::RGBA ? 4 : 3;
		if (size != bytesPerPixel * m_Specification.Width * m_Specification.Height)
		{
			OHM_ERROR("Data size must match entire texture.");
			return;
		}

		GLenum pixelLayout = ConverDataLayoutMode(m_Specification.PixelLayoutFormat);
		GLenum type = ConvertImageDataType(m_Specification.DataType);
		glTextureSubImage2D(m_ID, 0, 0, 0, m_Specification.Width, m_Specification.Height, pixelLayout, type, data);
	}

	void Texture2D::ClearBinding()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	Ref<Texture2D> Texture2D::CreateWhiteTexture()
	{
		Texture2DSpecification whiteTextureSpec =
		{
			TextureUtils::WrapMode::Repeat,
			TextureUtils::WrapMode::Repeat,
			TextureUtils::FilterMode::Linear,
			TextureUtils::FilterMode::Linear,
			TextureUtils::ImageInternalFormat::RGBA8,
			TextureUtils::ImageDataLayout::RGBA,
			TextureUtils::ImageDataType::UByte,
			1, 1,
			"White Texture"
		};

		Ref<Texture2D> whiteTexture = CreateRef<Texture2D>(whiteTextureSpec);
		uint32_t whiteTextureData = 0xffffffff;
		whiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		return whiteTexture;
	}

	Ref<Texture2D> Texture2D::CreateBlackTexture()
	{
		Texture2DSpecification blackTextureSpec =
		{
			TextureUtils::WrapMode::Repeat,
			TextureUtils::WrapMode::Repeat,
			TextureUtils::FilterMode::Linear,
			TextureUtils::FilterMode::Linear,
			TextureUtils::ImageInternalFormat::RGBA8,
			TextureUtils::ImageDataLayout::RGBA,
			TextureUtils::ImageDataType::UByte,
			1, 1,
			"Black Texture"
		};

		Ref<Texture2D> blackTexture = CreateRef<Texture2D>(blackTextureSpec);
		uint32_t blackTextureData = 0xff000000;
		blackTexture->SetData(&blackTextureData, sizeof(uint32_t));
		return blackTexture;
	}
}