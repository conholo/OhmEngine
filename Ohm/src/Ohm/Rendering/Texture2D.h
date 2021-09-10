#pragma once

namespace Ohm
{
	typedef unsigned int GLenum;

	class Texture2D
	{
	public:
		Texture2D(const std::string& filePath);
		Texture2D(uint32_t width, uint32_t height);
		~Texture2D();

		void Bind(uint32_t textureSlot) const;
		void SetData(const void* data, uint32_t size);

		uint32_t GetID() const { return m_ID; }

	private:
		uint32_t m_ID;
		uint32_t m_Width, m_Height;
		GLenum m_InternalFormat, m_DataFormat;
	};
}