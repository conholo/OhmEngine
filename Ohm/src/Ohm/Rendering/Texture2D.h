#pragma once

#include "Ohm/Rendering/Utility/TextureUtils.h"

namespace Ohm
{
	struct Texture2DSpecification
	{
		TextureUtils::WrapMode SamplerWrap = TextureUtils::WrapMode::Repeat;
		TextureUtils::FilterMode MinFilter = TextureUtils::FilterMode::Linear;
		TextureUtils::FilterMode MagFilter = TextureUtils::FilterMode::Linear;
		TextureUtils::ImageInternalFormat InternalFormat = TextureUtils::ImageInternalFormat::RGBA8;
		TextureUtils::ImageDataLayout DataLayout = TextureUtils::ImageDataLayout::RGBA;
		TextureUtils::ImageDataType DataType = TextureUtils::ImageDataType::UByte;
		uint32_t Width, Height;
	};

	class Texture2D
	{
	public:
		Texture2D(const std::string& filePath, const Texture2DSpecification& specification);
		Texture2D(const Texture2DSpecification& specification);
		~Texture2D();

		void BindToSamplerSlot(uint32_t unit) const;
		void UnbindFromSamplerSlot(uint32_t unit = 0) const;

		void SetData(const void* data, uint32_t size);
		void Invalidate();
		void BindToImageSlot(uint32_t unit, uint32_t level, TextureUtils::TextureAccessLevel access, TextureUtils::TextureShaderDataFormat shaderDataFormat);
		void Resize(uint32_t width, uint32_t height);

		uint32_t GetWidth() const { return m_Specification.Width; }
		uint32_t GetHeight() const { return m_Specification.Height; }

		uint32_t GetMipCount() const { return TextureUtils::CalculateMipLevelCount(m_Specification.Width, m_Specification.Height); }
		uint32_t GetID() const { return m_ID; }
		std::pair<uint32_t, uint32_t> GetMipDimensions(uint32_t mip) const;

	private:
		Texture2DSpecification m_Specification;
		uint32_t m_ID;
	};
}