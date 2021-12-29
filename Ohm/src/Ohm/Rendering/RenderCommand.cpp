#include "ohmpch.h"
#include "Ohm/Rendering/RenderCommand.h"

#include <glad/glad.h>

namespace Ohm
{
	void OpenGLMessageCallback(
		unsigned source,
		unsigned type,
		unsigned id,
		unsigned severity,
		int length,
		const char* message,
		const void* userParam
	)
	{
		switch (severity)
		{
			case GL_DEBUG_SEVERITY_HIGH:         OHM_CORE_CRITICAL(message); return;
			case GL_DEBUG_SEVERITY_MEDIUM:       OHM_CORE_ERROR(message); return;
			case GL_DEBUG_SEVERITY_LOW:          OHM_CORE_WARN(message); return;
			case GL_DEBUG_SEVERITY_NOTIFICATION: OHM_CORE_TRACE(message); return;
		}
	}

	void RenderCommand::Initialize()
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLMessageCallback, nullptr);

		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);

		//glEnable(GL_CULL_FACE);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	void RenderCommand::SetFlags(uint32_t flags)
	{
		if (flags & (uint32_t)RenderFlag::DepthTest)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);

		if (flags & (uint32_t)RenderFlag::Blend)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
	}

	void RenderCommand::Clear(bool colorBufferBit, bool depthBufferBit)
	{
		if (colorBufferBit && depthBufferBit)
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
		else
		{
			if (colorBufferBit)
			{
				glClear(GL_COLOR_BUFFER_BIT);
			}
			else if (depthBufferBit)
			{
				glClear(GL_DEPTH_BUFFER_BIT);
			}
		}
	}

	void RenderCommand::SetViewport(uint32_t width, uint32_t height)
	{
		glViewport(0, 0, width, height);
	}

	void RenderCommand::ClearColor(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);
	}

	void RenderCommand::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
	}

	void RenderCommand::SetDepthFlag(DepthFlag depthFlag)
	{
		switch (depthFlag)
		{
			case DepthFlag::Never:
			{
				glDepthFunc(GL_NEVER);
				break;
			}
			case DepthFlag::Less:
			{
				glDepthFunc(GL_LESS);
				break;
			}
			case DepthFlag::Equal:
			{
				glDepthFunc(GL_EQUAL);
				break;
			}
			case DepthFlag::LEqual:
			{
				glDepthFunc(GL_LEQUAL);
				break;
			}
			case DepthFlag::Greater:
			{
				glDepthFunc(GL_GREATER);
				break;
			}
			case DepthFlag::NotEqual:
			{
				glDepthFunc(GL_NOTEQUAL);
				break;
			}
			case DepthFlag::GEqual:
			{
				glDepthFunc(GL_GEQUAL);
				break;
			}
			case DepthFlag::Always:
			{
				glDepthFunc(GL_ALWAYS);
				break;
			}
		}
	}
}