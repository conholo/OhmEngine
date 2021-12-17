#include "ohmpch.h"
#include "Ohm/Rendering/Texture2D.h"

#include <glad/glad.h>
#include <stb_image.h>

namespace Ohm
{
	Texture2D::Texture2D(const std::string& filePath, const Texture2DSpecification& specification)
		:m_Specification(specification), m_FilePath(filePath)
	{
		size_t pos = m_FilePath.find_last_of("/") + 1;
		size_t size = m_FilePath.size() - pos;
		m_Name = m_FilePath.substr(pos, size);
		glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
		glBindTexture(GL_TEXTURE_2D, m_ID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, TextureUtils::ConvertWrapMode(specification.SamplerWrap));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, TextureUtils::ConvertWrapMode(specification.SamplerWrap));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TextureUtils::ConvertMinMagFilterMode(specification.MinFilter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TextureUtils::ConvertMinMagFilterMode(specification.MagFilter));

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
		:m_Specification(specification), m_FilePath(""), m_Name(specification.Name)
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

	void Texture2D::BindToSamplerSlot(uint32_t unit)
	{
		glBindTextureUnit(unit, m_ID);
	}

	void Texture2D::UnbindFromSamplerSlot(uint32_t unit)
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

	void TextureLibrary::Add(const Ref<Texture2D>& texture)
	{
		if (s_TextureLibrary.find(texture->GetName()) == s_TextureLibrary.end())
		{
			s_TextureLibrary[texture->GetName()] = texture;
			s_TextureLibraryIDs[texture->GetID()] = texture;
			OHM_CORE_TRACE("Added {} Texture to the Texture Library.", texture->GetName());
		}
		else
		{
			OHM_CORE_WARN("Texture already contained in Shader Library.  Attempted to add {} Texture to Library.", texture->GetName());
		}
	}

	Ref<Texture2D> TextureLibrary::Load(const Texture2DSpecification& spec, const std::string& filePath)
	{
		if (!filePath.empty())
		{
			Ref<Texture2D> texture = CreateRef<Texture2D>(filePath, spec);
			Add(texture);
			return texture;
		}

		Ref<Texture2D> texture = CreateRef<Texture2D>(spec);
		Add(texture);
		return texture;
	}

	Ref<Texture2D> TextureLibrary::Load(const std::string& filePath)
	{
		Texture2DSpecification defaultFromFileSpec =
		{
			"",
			TextureUtils::WrapMode::Repeat,
			TextureUtils::FilterMode::LinearMipLinear,
			TextureUtils::FilterMode::Linear,
			TextureUtils::ImageInternalFormat::FromImage,
			TextureUtils::ImageDataLayout::FromImage,
			TextureUtils::ImageDataType::UByte,
		};

		Ref<Texture2D> texture = CreateRef<Texture2D>(filePath, defaultFromFileSpec);
		Add(texture);

		return texture;
	}

	const Ref<Texture2D>& TextureLibrary::Get(const std::string& name)
	{
		if (s_TextureLibrary.find(name) == s_TextureLibrary.end())
		{
			OHM_CORE_ERROR("No texture with name \"{}\" found in Texture Library.", name);
			return nullptr;
		}

		return s_TextureLibrary.at(name);
	}

	Ref<Texture2D> TextureLibrary::GetTextureAt(uint32_t queryBinding)
	{
		for (auto [name, texture] : s_TextureLibrary)
		{
			if (texture->GetID() != queryBinding) continue;

			return texture;
		}

		return nullptr;
	}

	std::unordered_map<std::string, int32_t> TextureLibrary::BindAndGetMaterialTextureSlots(const std::unordered_map<std::string, uint32_t>& textureIDs)
	{
		std::unordered_map<std::string, int32_t> nameToSlotMap;
		uint32_t currentSlot = 1;
		for (auto [name, id] : textureIDs)
		{
			if (s_TextureLibraryIDs.find(id) == s_TextureLibraryIDs.end())
			{
				// THIS NEEDS TO BE UPDATED TO HANDLE TEXTURES THAT HAVEN'T BEEN REGISTERED!
				//OHM_WARN("Texture with name: '{}' and id: '{}' could not be found in the Texture Library.  Could not bind this texture.  Check that it has been added to the Texture Library.", name, id);
				//continue;
			}

			if (name == "sampler_ShadowMap")
			{
				nameToSlotMap[name] = 0;
				glBindTextureUnit(0, id);
			}
			else
			{
				nameToSlotMap[name] = currentSlot;
				glBindTextureUnit(currentSlot++, s_TextureLibraryIDs[id]->GetID());
			}
		}

		return nameToSlotMap;
	}


	std::unordered_map<std::string, Ref<Texture2D>> TextureLibrary::s_TextureLibrary;
	std::unordered_map<uint32_t, Ref<Texture2D>> TextureLibrary::s_TextureLibraryIDs;
}