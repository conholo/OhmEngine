#pragma once


namespace Ohm
{
	enum class FramebufferTextureFormat
	{
		None = 0,
		RGBA8,
		RED_INTEGER,
		DEPTH24STENCIL8,

		Depth = DEPTH24STENCIL8
	};


	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification(FramebufferTextureFormat format)
			: TextureFormat(format) { }

		FramebufferTextureFormat TextureFormat;
	};

	
	struct FramebufferAttachmentData
	{
		FramebufferAttachmentData() = default;
		FramebufferAttachmentData(std::initializer_list<FramebufferTextureSpecification> textureSpecifications)
			: TextureSpecifications(textureSpecifications) { }

		std::vector<FramebufferTextureSpecification> TextureSpecifications;
	};


	struct FramebufferSpecification
	{
		uint32_t Width = 0, Height = 0;
		uint32_t Samples = 1;

		FramebufferAttachmentData AttachmentData;
	};


	class Framebuffer
	{
	public:
		Framebuffer(const FramebufferSpecification& specification);
		~Framebuffer();

		void Bind() const;
		void Unbind() const;

		void Invalidate();

		void Resize(uint32_t width, uint32_t height);
		int ReadPixel(uint32_t attachmentIndex, int x, int y);

		void ClearAttachment(uint32_t attachmentIndex, int value);
		uint32_t GetColorAttachmentID(uint32_t index = 0) const { return m_ColorAttachmentIDs[index]; }

		const FramebufferSpecification& GetFrameBufferSpecification() const { return m_Specification; }

	private:
		uint32_t m_ID;
		FramebufferSpecification m_Specification;

		std::vector<FramebufferTextureSpecification> m_ColorTextureSpecifications;
		FramebufferTextureSpecification m_DepthTextureSpecification;

		std::vector<uint32_t> m_ColorAttachmentIDs;
		uint32_t m_DepthAttachmentID;
	};
}