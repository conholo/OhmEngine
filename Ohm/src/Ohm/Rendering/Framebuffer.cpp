#include "ohmpch.h"
#include "Ohm/Rendering/Framebuffer.h"
#include "Ohm/Rendering/Utility/TextureUtils.h"
#include <glad/glad.h>

#define TINYEXR_IMPLEMENTATION
#include <tinyexr.h>

namespace Ohm
{
	static GLenum TextureFormatToGLenum(FramebufferTextureFormat format)
	{
		switch (format)
		{
			case FramebufferTextureFormat::RGBA32F:
			case FramebufferTextureFormat::RGBA8: 
			{
				return GL_RGBA;
			}
			case FramebufferTextureFormat::RED_INTEGER: 
			{
				return GL_RED_INTEGER;
			}
			case FramebufferTextureFormat::DEPTH24STENCIL8: 
			{
				return GL_DEPTH24_STENCIL8;
			}
			default:	return 0;
		}
	}

	static void AttachColorTexture(uint32_t attachmentId, GLenum internalFormat, GLenum dataFormat, GLenum dataType, uint32_t width, uint32_t height, uint32_t index)
	{
		uint32_t mips = TextureUtils::CalculateMipLevelCount(width, height);
		glTextureStorage2D(attachmentId, mips, internalFormat, width, height);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, attachmentId, 0);
	}

	static void AttachDepthTexture(uint32_t attachmentId, GLenum internalFormat, GLenum depthAttachmentType, uint32_t width, uint32_t height)
	{
		glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, depthAttachmentType, GL_TEXTURE_2D, attachmentId, 0);
	}

	static void AttachLayeredDepthTarget(uint32_t AttachmentID, GLenum InternalFormat, uint32_t Width, uint32_t Height, uint32_t Layers)
	{
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, InternalFormat, Width, Height, Layers, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		constexpr float BorderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, BorderColor);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, AttachmentID, 0);
	}

	static bool IsDepthFormat(const FramebufferTextureFormat Format)
	{
		return Format == FramebufferTextureFormat::DEPTH24STENCIL8  || Format == FramebufferTextureFormat::DEPTH32F;
	}
	
	Framebuffer::Framebuffer(FramebufferSpecification spec)
		:m_Specification(std::move(spec))
	{
		for (auto textureFormatSpecification : m_Specification.AttachmentSpecification.FBOTextureSpecifications)
		{
			if (!IsDepthFormat(textureFormatSpecification.TextureFormat))
				m_ColorAttachmentTextureSpecs.emplace_back(textureFormatSpecification);
			else
				m_DepthAttachmentTextureSpec = textureFormatSpecification;
		}

		Invalidate();
	}

	Framebuffer::~Framebuffer()
	{
		glDeleteFramebuffers(1, &m_ID);
	}

	void Framebuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_ID);
		glViewport(0, 0, m_Specification.Width, m_Specification.Height);
	}

	void Framebuffer::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Framebuffer::Invalidate()
	{
		if (m_ID)
		{
			glDeleteFramebuffers(1, &m_ID);
			glDeleteTextures(m_ColorAttachmentIDs.size(), m_ColorAttachmentIDs.data());
			glDeleteTextures(1, &m_DepthAttachmentID);

			m_ColorAttachmentIDs.clear();
			m_DepthAttachmentID = 0;
		}

		glCreateFramebuffers(1, &m_ID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_ID);

		if (!m_ColorAttachmentTextureSpecs.empty())
		{
			m_ColorAttachmentIDs.resize(m_ColorAttachmentTextureSpecs.size());

			glCreateTextures(GL_TEXTURE_2D, m_ColorAttachmentIDs.size(), m_ColorAttachmentIDs.data());

			for (size_t i = 0; i < m_ColorAttachmentIDs.size(); i++)
			{
				glBindTexture(GL_TEXTURE_2D, m_ColorAttachmentIDs[i]);

				switch (m_ColorAttachmentTextureSpecs[i].TextureFormat)
				{
					case FramebufferTextureFormat::RGBA8:
					{
						AttachColorTexture(m_ColorAttachmentIDs[i], GL_RGBA8, GL_RGBA, GL_UNSIGNED_INT, m_Specification.Width, m_Specification.Height, i);
						break;
					}
					case FramebufferTextureFormat::RGBA32F:
					{
						AttachColorTexture(m_ColorAttachmentIDs[i], GL_RGBA32F, GL_RGBA, GL_FLOAT, m_Specification.Width, m_Specification.Height, i);
						break;
					}
					case FramebufferTextureFormat::RED_INTEGER:
					{
						AttachColorTexture(m_ColorAttachmentIDs[i], GL_R32I, GL_RED_INTEGER, GL_INT, m_Specification.Width, m_Specification.Height, i);
						break;
					}
				default: break;
				}
			}
		}

		if (m_DepthAttachmentTextureSpec.TextureFormat != FramebufferTextureFormat::None)
		{
			if(m_Specification.IsLayered)
			{
				glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_DepthAttachmentID);
				glBindTexture(GL_TEXTURE_2D_ARRAY, m_DepthAttachmentID);
				AttachLayeredDepthTarget(m_DepthAttachmentID, GL_DEPTH_COMPONENT32F, m_Specification.Width, m_Specification.Height, m_Specification.Layers);
			}
			else
			{
				glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthAttachmentID);
				glBindTexture(GL_TEXTURE_2D, m_DepthAttachmentID);

				if(m_DepthAttachmentTextureSpec.TextureFormat == FramebufferTextureFormat::DEPTH24STENCIL8)
					AttachDepthTexture(m_DepthAttachmentID, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.Width, m_Specification.Height);
				else if(m_DepthAttachmentTextureSpec.TextureFormat == FramebufferTextureFormat::DEPTH32F)
					AttachDepthTexture(m_DepthAttachmentID, GL_DEPTH_COMPONENT32F, GL_DEPTH_ATTACHMENT, m_Specification.Width, m_Specification.Height);
			}
		}

		if (m_ColorAttachmentIDs.size() > 1)
		{
			ASSERT(m_ColorAttachmentIDs.size() <= 4, "4 Color attachments is the max # of attachments per FBO.");
			GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
			glDrawBuffers(m_ColorAttachmentIDs.size(), buffers);
		}
		else if (m_ColorAttachmentIDs.empty())
		{
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}

		const bool CompleteFBO = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
		ASSERT(CompleteFBO, "Framebuffer Incomplete.")
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Framebuffer::Resize(uint32_t width, uint32_t height)
	{
		m_Specification.Width = width;
		m_Specification.Height = height;

		Invalidate();
	}

	void Framebuffer::BindDepthTexture(uint32_t slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTextureUnit(slot, m_DepthAttachmentID);
	}

	void Framebuffer::BindColorAttachment(uint32_t index, uint32_t slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTextureUnit(slot, m_ColorAttachmentIDs[index]);
	}

	void Framebuffer::BindColorAttachmentToImageSlot(uint32_t unit, uint32_t level, TextureUtils::TextureAccessLevel access, TextureUtils::TextureShaderDataFormat shaderDataFormat, uint32_t index) const
	{
		const GLenum glShaderDataFormat = ConvertShaderFormatType(shaderDataFormat);
		const GLenum internalFormat = ConvertInternalFormatMode(TextureUtils::ImageInternalFormat::RGBA32F);

		if (glShaderDataFormat != internalFormat)
		{
			OHM_CORE_ERROR("Shader Data Format and Internal format must match!");
			return;
		}

		glBindImageTexture(unit, m_ColorAttachmentIDs[index], level, GL_FALSE, 0, ConvertTextureAccessLevel(access), ConvertShaderFormatType(shaderDataFormat));
	}

	void Framebuffer::UnbindColorAttachment(uint32_t index, uint32_t slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTextureUnit(slot, 0);
	}

	void Framebuffer::ReadColorData(void* pixels, uint32_t attachmentIndex) const
	{
		ASSERT(attachmentIndex <= m_ColorAttachmentIDs.size() - 1, "Invalid attachment index to read color data from.");
		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex); 
		glReadPixels(0, 0, m_Specification.Width, m_Specification.Height, GL_RGB, GL_FLOAT, pixels);
	}

	void Framebuffer::ClearAttachment(uint32_t attachmentIndex, int value) const
	{
		ASSERT(attachmentIndex <= m_ColorAttachmentIDs.size(), "FBO Error: Can't clear color attachment - No attachment at index: {}", attachmentIndex);
		auto& specification = m_ColorAttachmentTextureSpecs[attachmentIndex];
		const uint32_t texID = m_ColorAttachmentIDs[attachmentIndex];
		const GLenum format = TextureFormatToGLenum(specification.TextureFormat);
		glClearTexImage(texID, 0, format, GL_INT, &value);
	}

	uint32_t Framebuffer::GetColorAttachmentID(uint32_t index) const
	{
		ASSERT(index <= m_ColorAttachmentIDs.size(), "FBO Error: No color attachment at index: {}", index);
		return m_ColorAttachmentIDs[index]; 
	}

	template <class t>
	void flip_vertical(const int rows, const int columns, t* data)  // flips: bottom-up
	{
		for (int c = 0; c < columns; ++c)
		{
			for (int r = 0; r < rows / 2; ++r)
			{
				for (int k = 0; k < 3; ++k)
					std::swap<t>(data[(c + r * columns) * 3 + k], data[(c + (rows - 1 - r) * columns) * 3 + k]);
			}
		}
	}
	
	bool Framebuffer::SaveAttachmentAsEXR(const std::string& fileName, uint32_t attachmentIndex)
	{
		std::ifstream f(fileName.c_str());
		if(!f.good())
		{
			std::ofstream created(fileName.c_str());
			if(created)
				created.close();
		}

		const glm::vec2 viewport = GetCurrentSize();
		const size_t sizeInBytes = (size_t)viewport.x * (size_t)viewport.y * 3;
		auto* data = new float[sizeInBytes];

		Bind();
		ReadColorData(data, attachmentIndex);
		Unbind();

		flip_vertical<float>(viewport.y, viewport.x, data);

		const char* err = nullptr;
		const int   ret = SaveEXR(data,
			viewport.x,
			viewport.y,
			3,
			0,
			fileName.c_str(),
			&err);
		const bool saved = ret == TINYEXR_SUCCESS;
		delete[] data;
		FreeEXRErrorMessage(err);

		return saved;
	}
}
