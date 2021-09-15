#pragma once

#include "Ohm/Scene/Component.h"
#include "Ohm/Rendering/EditorCamera.h"
#include "Ohm/Rendering/RenderPass.h"
#include "Ohm/Rendering/Shader.h"
#include "Ohm/Rendering/Mesh.h"

namespace Ohm
{
	class Renderer
	{
	public:
		static void Initialize();
		static void BeginScene();
		static void BeginPass(const Ref<RenderPass>& renderPass);
		static void DrawMeshWithMaterial(const EditorCamera& camera, MeshRendererComponent& meshRenderer, const TransformComponent& transform);
		static void DrawGeometry(MeshRendererComponent& meshRenderer);
		static void DrawFullScreenQuad();
		static void EndPass(const Ref<RenderPass>& renderPass);
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