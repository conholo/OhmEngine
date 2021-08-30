#pragma once

#include "Ohm/Rendering/VertexArray.h"

namespace Ohm
{
	class RenderCommand
	{
	public:

		static void Initialize();
		static void Clear();
		static void SetViewport(uint32_t width, uint32_t height);
		static void ClearColor(float r, float g, float b, float a);
		static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount);
	};
}