#pragma once

#include <glm/glm.hpp>
#include "Ohm.h"

namespace Ohm
{
	class Viewport
	{
	public:
		Viewport() = default;
		Viewport(const Ref<Framebuffer>& framebuffer);

		void SetFramebuffer(const Ref<Framebuffer>& framebuffer) { m_Framebuffer = framebuffer; }

		void Draw();

		const glm::vec2& GetViewportSize() const { return m_ViewportSize; }
		const glm::vec2& GetViewportBoundsMin() const { return m_ViewportBoundsMin; }
		const glm::vec2& GetViewportBoundsMax() const { return m_ViewportBoundsMax; }

	private:
		Ref<Framebuffer> m_Framebuffer;
		glm::vec2 m_ViewportSize{ 0.0f };
		glm::vec2 m_ViewportBoundsMin{ 0.0f };
		glm::vec2 m_ViewportBoundsMax{ 0.0f };
	};
}

