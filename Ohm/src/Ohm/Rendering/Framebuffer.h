#pragma once
#include <vector>
#include <glm/glm.hpp>

namespace Ohm
{
	namespace TextureUtils
	{
		enum class TextureShaderDataFormat;
		enum class TextureAccessLevel;
	}

	enum class FramebufferTextureFormat
	{
		None = 0,
		RGBA8,
		RGBA32F,
		RED_INTEGER,
		DEPTH24STENCIL8,
		DEPTH32F,

		Depth = DEPTH24STENCIL8,
		Depth32F = DEPTH32F,
	};

	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification(FramebufferTextureFormat format)
			: TextureFormat(format) { }

		FramebufferTextureFormat TextureFormat;
	};

	struct FramebufferAttachmentSpecification
	{
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> fboTextureSpecifications)
			:FBOTextureSpecifications(fboTextureSpecifications) { }

		std::vector<FramebufferTextureSpecification> FBOTextureSpecifications;
	};

	struct FramebufferSpecification
	{
		uint32_t Width = 0, Height = 0;
		FramebufferAttachmentSpecification AttachmentSpecification;
		bool IsLayered = false;
		uint32_t Layers = 0;
	};

	class Framebuffer
	{
	public:
		Framebuffer(FramebufferSpecification spec);
		~Framebuffer();

		void Bind() const;
		void Unbind() const;

		void Invalidate();
		void Resize(uint32_t width, uint32_t height);

		void BindDepthTexture(uint32_t slot = 0) const;
		void BindColorAttachment(uint32_t index = 0, uint32_t slot = 0) const;
		void BindColorAttachmentToImageSlot(uint32_t unit, uint32_t level, TextureUtils::TextureAccessLevel access, TextureUtils::TextureShaderDataFormat shaderDataFormat, uint32_t index = 0) const;

		void UnbindColorAttachment(uint32_t index, uint32_t slot) const;
		void ClearAttachment(uint32_t attachmentIndex, int value) const;
		uint32_t GetColorAttachmentID(uint32_t index = 0) const;
		uint32_t GetDepthAttachmentID() const { return m_DepthAttachmentID; }
		uint32_t GetLayeredDepthAttachmentID() const { return m_DepthAttachmentID; };

		void ReadColorData(void* pixels, uint32_t attachmentIndex) const;
		
		const FramebufferSpecification& GetFramebufferSpecification() const { return m_Specification; }
		glm::vec2 GetCurrentSize() { return {m_Specification.Width, m_Specification.Height}; }
		bool SaveAttachmentAsEXR(const std::string& fileName, uint32_t attachmentIndex);
		
	private:
		FramebufferSpecification m_Specification;
		uint32_t m_ID;

		std::vector<FramebufferTextureSpecification> m_ColorAttachmentTextureSpecs;
		std::vector<uint32_t> m_ColorAttachmentIDs;

		FramebufferTextureSpecification m_DepthAttachmentTextureSpec{ FramebufferTextureFormat::None };
		uint32_t m_DepthAttachmentID;
	};
}