#include "ohmpch.h"
#include "Ohm/Rendering/FBO.h"

#include <glad/glad.h>

namespace Ohm
{
	FBO::FBO(uint32_t width, uint32_t height)
		:m_Width(width), m_Height(height)
	{
		Invalidate();
	}

	void FBO::Invalidate()
	{
		if (m_ID)
		{
			glDeleteFramebuffers(1, &m_ID);
			glDeleteTextures(1, &m_ColorAttachmentID);
		}

		glCreateFramebuffers(1, &m_ID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_ID);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorAttachmentID);
		glBindTexture(GL_TEXTURE_2D, m_ColorAttachmentID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachmentID, 0);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthAttachmentID);
		glBindTexture(GL_TEXTURE_2D, m_DepthAttachmentID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_Width, m_Height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachmentID, 0);


		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void FBO::Resize(uint32_t width, uint32_t height)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_ID);
		m_Width = width;
		m_Height = height;
		Invalidate();
	}

	void FBO::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_ID);
	}

	void FBO::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	FBO::~FBO()
	{
		glDeleteFramebuffers(1, &m_ID);
	}

}