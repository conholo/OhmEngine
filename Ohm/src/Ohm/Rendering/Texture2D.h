#pragma once

#include "Ohm/Rendering/Utility/TextureUtils.h"

namespace Ohm
{
	struct Texture2DSpecification
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

	class Texture2D
	{
	public:
		Texture2D(const std::string& filePath, const Texture2DSpecification& specification);
		Texture2D(const Texture2DSpecification& specification);
		~Texture2D();

		void BindToSamplerSlot(uint32_t unit);
		void UnbindFromSamplerSlot(uint32_t unit = 0);

		void SetData(const void* data, uint32_t size);
		void Invalidate();
		void BindToImageSlot(uint32_t unit, uint32_t level, TextureUtils::TextureAccessLevel access, TextureUtils::TextureShaderDataFormat shaderDataFormat);
		void Resize(uint32_t width, uint32_t height);

		uint32_t GetWidth() const { return m_Specification.Width; }
		uint32_t GetHeight() const { return m_Specification.Height; }

		uint32_t GetMipCount() const { return TextureUtils::CalculateMipLevelCount(m_Specification.Width, m_Specification.Height); }
		uint32_t GetID() const { return m_ID; }
		std::string GetName() const { return m_Name; }
		std::string GetFilePath() const { return m_FilePath; }
		std::pair<uint32_t, uint32_t> GetMipDimensions(uint32_t mip) const;

	private:
		Texture2DSpecification m_Specification;
		std::string m_FilePath;
		std::string m_Name;
		uint32_t m_ID;
		int32_t m_Unit;
	};

	class TextureLibrary
	{
	public:
		static void Add(const Ref<Texture2D>& texture);
		static Ref<Texture2D> Load(const Texture2DSpecification& spec, const std::string& filePath = "");
		static Ref<Texture2D> Load(const std::string& filePath = "");
		static const Ref<Texture2D>& Get(const std::string& name);
		static Ref<Texture2D> GetTextureAt(uint32_t unit);
		static std::unordered_map<std::string, int32_t> BindAndGetMaterialTextureSlots(const std::unordered_map<std::string, uint32_t>& textureIDs);

		static std::unordered_map<std::string, Ref<Texture2D>> GetLibrary() { return s_TextureLibrary; }

	private:
		static std::unordered_map<std::string, Ref<Texture2D>> s_TextureLibrary;
		static std::unordered_map<uint32_t, Ref<Texture2D>> s_TextureLibraryIDs;
	};
}