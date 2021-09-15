#pragma once

#include "Ohm/Rendering/Shader.h"
#include "Ohm/Rendering/Framebuffer.h"
#include <glm/glm.hpp>

namespace Ohm
{
	struct RenderPassSpecification
	{
		Ref<Framebuffer> TargetFramebuffer;
		Ref<Shader> Shader;
		bool DepthWrite = true;
		bool DepthRead = true;
		bool ColorWrite = true;
		glm::vec4 ClearColor{ 0.0f };
	};

	class RenderPass
	{
	public:
		RenderPass(const RenderPassSpecification& specification);

		RenderPassSpecification& GetRenderPassSpecification() { return m_Specification; }
		const RenderPassSpecification& GetRenderPassSpecification() const { return m_Specification; }

	private:
		RenderPassSpecification m_Specification;
	};
}