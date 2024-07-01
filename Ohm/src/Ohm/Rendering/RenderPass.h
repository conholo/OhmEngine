#pragma once

#include "Ohm/Rendering/Material.h"
#include "Ohm/Rendering/Framebuffer.h"
#include <glm/glm.hpp>

namespace Ohm
{
	enum class PassType { None = 0, DefaultFBO, CustomFBO };

	struct RenderPassSpecification
	{
		PassType Type = PassType::CustomFBO;
		Ref<Framebuffer> TargetFramebuffer = nullptr;
		Ref<Material> PassMaterial = nullptr;
		uint32_t Flags {};

		bool ClearDepthFlag = true;
		bool ClearColorFlag = true;
		glm::vec4 ClearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
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