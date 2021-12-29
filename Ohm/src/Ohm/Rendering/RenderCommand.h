#pragma once

#include "Ohm/Rendering/VertexArray.h"

namespace Ohm
{
	enum class RenderFlag
	{
		None		= 1 << 0,
		DepthTest	= 1 << 1,
		Blend		= 1 << 2,
		TwoSided	= 1 << 3,
	};

	enum class DepthFlag
	{
		Never,
		Less,
		Equal,
		LEqual,
		Greater,
		NotEqual,
		GEqual,
		Always
	};

	class RenderCommand
	{
	public:
		static void Initialize();
		static void SetFlags(uint32_t flags);
		static void Clear(bool colorBufferBit, bool depthBufferBit);
		static void SetViewport(uint32_t width, uint32_t height);
		static void ClearColor(float r, float g, float b, float a);
		static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount);
		static void SetDepthFlag(DepthFlag depthFlag);
	private:
		static uint32_t s_DepthFlags;
	};
}