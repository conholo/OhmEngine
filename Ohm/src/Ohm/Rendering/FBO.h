#pragma once

namespace Ohm
{
	class FBO
	{
	public:
		FBO(uint32_t width, uint32_t height);
		~FBO();

		uint32_t GetColorAttachmentID() const { return m_ColorAttachmentID; }
		uint32_t GetDepthAttachmentID() const { return m_DepthAttachmentID; }
		uint32_t Width() const { return m_Width; }
		uint32_t Height() const { return m_Height; }

		void Invalidate();

		void Resize(uint32_t width, uint32_t height);


		void Bind() const;
		void Unbind() const;

	private:
		uint32_t m_Width, m_Height;
		uint32_t m_ColorAttachmentID;
		uint32_t m_DepthAttachmentID;
		uint32_t m_ID;
	};
}
