#pragma once
#include "Ohm/Rendering/Utility/TextureUtils.h"


namespace Ohm
{
	struct TextureCubeSpecification
	{
		TextureUtils::WrapMode SamplerWrapS = TextureUtils::WrapMode::ClampToEdge;
		TextureUtils::WrapMode SamplerWrapT = TextureUtils::WrapMode::ClampToEdge;
		TextureUtils::WrapMode SamplerWrapR = TextureUtils::WrapMode::ClampToEdge;
		TextureUtils::FilterMode MinFilter = TextureUtils::FilterMode::LinearMipLinear;
		TextureUtils::FilterMode MagFilter = TextureUtils::FilterMode::Linear;
		TextureUtils::ImageInternalFormat InternalFormat = TextureUtils::ImageInternalFormat::RGBA32F;
		TextureUtils::ImageDataLayout DataLayout = TextureUtils::ImageDataLayout::RGBA;
		TextureUtils::ImageDataType DataType = TextureUtils::ImageDataType::Float;
		uint32_t Dimension = 512;
		std::string Name = "TextureCube";
	};

	class TextureCube
	{
	public:
		TextureCube(const TextureCubeSpecification& spec, const std::vector<std::string>& cubeFaceFiles);
		TextureCube(const TextureCubeSpecification& spec);
		~TextureCube();

		void Invalidate(const TextureCubeSpecification& Specification);
		
		void BindToSamplerSlot(uint32_t slot = 0) const;
		static void Unbind();
		void BindToImageSlot(uint32_t Binding, uint32_t MipLevel, TextureUtils::TextureAccessLevel AccessLevel, TextureUtils::TextureShaderDataFormat ShaderDataFormat) const;

		void SetData(const void* data, size_t size) const;

		std::pair<uint32_t, uint32_t> GetMipSize(uint32_t Mip) const;
		uint32_t GetMipLevelCount() const;
		
		uint32_t GetID() const { return m_ID; }
		uint32_t GetDimension() const { return m_Specification.Dimension; }
		std::string GetName() const { return m_Name; }

	private:
		TextureCubeSpecification m_Specification;
		uint32_t m_ID;
		std::string m_Name;
	};
}