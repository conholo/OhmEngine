#pragma once
#include "Ohm/Rendering/Utility/TextureUtils.h"


namespace Ohm
{
	struct TextureCubeSpecification
	{
		std::string Name;
		TextureUtils::WrapMode SamplerWrap = TextureUtils::WrapMode::Repeat;
		TextureUtils::FilterMode MinFilter = TextureUtils::FilterMode::Linear;
		TextureUtils::FilterMode MagFilter = TextureUtils::FilterMode::Linear;
		TextureUtils::ImageInternalFormat InternalFormat = TextureUtils::ImageInternalFormat::RGBA8;
		TextureUtils::ImageDataLayout DataLayout = TextureUtils::ImageDataLayout::RGBA;
		TextureUtils::ImageDataType DataType = TextureUtils::ImageDataType::UByte;
		uint32_t Width, Height;
	};


	class TextureCube
	{
	public:
		TextureCube(const TextureCubeSpecification& spec);
		TextureCube(const TextureCubeSpecification& spec, const std::vector<std::string>& cubeFaceFiles);
		~TextureCube();

		void BindToSamplerSlot(uint32_t unit);
		void UnbindFromSamplerSlot(uint32_t unit = 0);

		void Invalidate();
		void BindToImageSlot(uint32_t unit, uint32_t level, TextureUtils::TextureAccessLevel access, TextureUtils::TextureShaderDataFormat shaderDataFormat, bool layered = false, uint32_t layer = 0);
		void Resize(uint32_t width, uint32_t height);
		void SetData(void* data, uint32_t size);
		void EnableShaderAccessBarrierBit();

		uint32_t GetWidth() const { return m_Specification.Width; }
		uint32_t GetHeight() const { return m_Specification.Height; }

		uint32_t GetMipCount() const { return TextureUtils::CalculateMipLevelCount(m_Specification.Width, m_Specification.Height); }
		uint32_t GetID() const { return m_ID; }
		std::string GetName() const { return m_Name; }
		std::string GetFilePath() const { return m_FilePath; }
		std::pair<uint32_t, uint32_t> GetMipDimensions(uint32_t mip) const;

	private:
		TextureCubeSpecification m_Specification;
		std::string m_FilePath;
		std::string m_Name;
		uint32_t m_ID;
		int32_t m_Unit;
	};
}