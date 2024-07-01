#pragma once

#include "Ohm/Rendering/Utility/TextureUtils.h"

namespace Ohm
{
	struct Texture2DSpecification
	{
		TextureUtils::WrapMode WrapModeS;
		TextureUtils::WrapMode WrapModeT;
		TextureUtils::FilterMode MinFilterMode;
		TextureUtils::FilterMode MagFilterMode;
		TextureUtils::ImageInternalFormat InternalFormat;
		TextureUtils::ImageDataLayout PixelLayoutFormat;
		TextureUtils::ImageDataType DataType;
		uint32_t Width, Height;
		std::string Name = "Texture2D";
	};

	class Texture2D
	{
	public:
		Texture2D(const Texture2DSpecification& specification);
		Texture2D(const Texture2DSpecification& specification, void* data);
		Texture2D(const std::string& filePath, const Texture2DSpecification& specification);
		~Texture2D();

		void Invalidate();
		void Clear() const;
		void Resize(uint32_t width, uint32_t height);

		uint32_t GetID() const { return m_ID; }
		const Texture2DSpecification& GetSpecification() const { return m_Specification; }

		std::pair<uint32_t, uint32_t> GetMipSize(uint32_t mip) const;
		uint32_t GetMipLevelCount() const;
		uint32_t GetWidth() const { return m_Specification.Width; }
		uint32_t GetHeight() const { return m_Specification.Height; }
		std::string GetName() const { return m_Name; }
		std::string GetFilePath() const { return m_FilePath; }

		static void BindTextureIDToSamplerSlot(uint32_t slot, uint32_t id);

		void BindToSamplerSlot(uint32_t slot) const;
		void Unbind(uint32_t slot = UINT32_MAX) const;
		void BindToImageSlot(uint32_t unit, uint32_t level, TextureUtils::TextureAccessLevel access, TextureUtils::TextureShaderDataFormat shaderDataFormat);
		void SetData(void* data, uint32_t size) const;
		static void ClearBinding();

		static Ref<Texture2D> CreateWhiteTexture();
		static Ref<Texture2D> CreateBlackTexture();

	private:
		Texture2DSpecification m_Specification;
		uint32_t m_ID;
		std::string m_FilePath;
		std::string m_Name;
	};
}