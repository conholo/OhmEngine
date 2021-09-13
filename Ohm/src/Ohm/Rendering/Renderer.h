#pragma once

#include "Ohm/Scene/Component.h"
#include "Ohm/Rendering/EditorCamera.h"

namespace Ohm
{
	class Renderer
	{
	public:
		static void Initialize();
		static void RemoveMesh(uint32_t id);
		static void BeginScene();
		static void Draw(const EditorCamera& camera, MeshRendererComponent& meshRenderer, const TransformComponent& transform);
		static void EndScene();
		static void Shutdown();

		struct Statistics
		{
			uint64_t TriangleCount;
			uint64_t VertexCount;

			void Clear()
			{
				TriangleCount = 0;
				VertexCount = 0;
			}
		};

		static Statistics GetStats() { return s_Stats; }

	private:
		static Statistics s_Stats;
	};
}