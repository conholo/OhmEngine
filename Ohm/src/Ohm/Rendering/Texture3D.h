#pragma once
#include "Utility/TextureUtils.h"
#include <string>

namespace Ohm
{
	struct Texture3DSpecification
	{
		TextureUtils::WrapMode WrapModeS;
		TextureUtils::WrapMode WrapModeT;
		TextureUtils::WrapMode WrapModeR;
		TextureUtils::FilterMode MinFilterMode;
		TextureUtils::FilterMode MagFilterMode;
		TextureUtils::ImageInternalFormat InternalFormat;
		TextureUtils::ImageDataLayout PixelLayoutFormat;
		TextureUtils::ImageDataType DataType;
		uint32_t Width, Height, Depth;
		std::string Name = "Texture3D";
	};

	class Texture3D
	{
	public:
		Texture3D(const Texture3DSpecification& specification);
		~Texture3D();

		void BindToSamplerSlot(uint32_t slot = 0) const;
		static void Unbind();
		void BindToImageSlot(uint32_t unit, uint32_t level, TextureUtils::TextureAccessLevel access, TextureUtils::TextureShaderDataFormat shaderDataFormat) const;

		void SetData(const void* data, size_t size);

		uint32_t GetID() const { return m_ID; }
		uint32_t GetWidth() const { return m_Specification.Width; }
		uint32_t GetHeight() const { return m_Specification.Height; }
		uint32_t GetDepth() const { return m_Specification.Depth; }
		std::string GetName() const { return m_Name; }
		std::string GetFilePath() const { return m_FilePath; }

		const Texture3DSpecification& GetSpecification() const { return m_Specification; }

		std::pair<uint32_t, uint32_t> GetMipSize(uint32_t mip) const;
		uint32_t GetMipLevelCount() const;
		
	private:
		Texture3DSpecification m_Specification;
		uint32_t m_ID;
		std::string m_FilePath;
		std::string m_Name;
	};
}
