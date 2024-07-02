#include "ohmpch.h"
#include "Ohm/Rendering/Texture3D.h"

#include "glad/glad.h"

namespace Ohm
{
    Texture3D::Texture3D(const Texture3DSpecification& specification)
		: m_Specification(specification), m_Name(specification.Name)
	{
		glCreateTextures(GL_TEXTURE_3D, 1, &m_ID);
		glBindTexture(GL_TEXTURE_3D, m_ID);

		const GLenum WrapS = ConvertWrapMode(specification.WrapModeS);
		const GLenum WrapT = ConvertWrapMode(specification.WrapModeT);
		const GLenum WrapR = ConvertWrapMode(specification.WrapModeR);
		const GLenum MinFilter = ConvertMinMagFilterMode(specification.MinFilterMode);
		const GLenum MagFilter = ConvertMinMagFilterMode(specification.MagFilterMode);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, WrapS);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, WrapT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, WrapR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, MinFilter);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, MagFilter);

		// Immutable-format Texture
		// Contents of the image can be modified, but it's storage requirements may not change.
		// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexStorage2D.xhtml
    	GLenum InternalFormat = ConvertInternalFormatMode(specification.InternalFormat);
    	glTextureStorage3D(m_ID, 1, InternalFormat, specification.Width, specification.Height, specification.Depth);
	}

	Texture3D::~Texture3D()
	{
		glDeleteTextures(1, &m_ID);
	}

	void Texture3D::BindToSamplerSlot(uint32_t slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTextureUnit(slot, m_ID);
	}

	void Texture3D::Unbind()
	{
		glBindTexture(GL_TEXTURE_3D, 0);
	}

	void Texture3D::BindToImageSlot(uint32_t unit, uint32_t level, TextureUtils::TextureAccessLevel access, TextureUtils::TextureShaderDataFormat shaderDataFormat) const
	{
		glBindImageTexture(unit, m_ID, level, GL_TRUE, 0, ConvertTextureAccessLevel(access), ConvertShaderFormatType(shaderDataFormat));
	}

	void Texture3D::SetData(const void* data, size_t size)
	{
    	uint32_t bytesPerPixel = m_Specification.PixelLayoutFormat == TextureUtils::ImageDataLayout::RGBA ? 4 : 3;
    	if (size != bytesPerPixel * m_Specification.Width * m_Specification.Height * m_Specification.Depth)
    	{
    		OHM_CORE_ERROR("Data size must match entire texture.");
    		return;
    	}

    	GLenum pixelLayout = ConverDataLayoutMode(m_Specification.PixelLayoutFormat);
    	GLenum type = ConvertImageDataType(m_Specification.DataType);
    	glTextureSubImage3D(m_ID, 0, 0, 0, 0, m_Specification.Width, m_Specification.Height, m_Specification.Depth, pixelLayout, type, data);
	}

	std::pair<glm::uint32_t, glm::uint32_t> Texture3D::GetMipSize(uint32_t mip) const
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

	uint32_t Texture3D::GetMipLevelCount() const
	{
		return TextureUtils::CalculateMipLevelCount(m_Specification.Width, m_Specification.Height);
	}
}
