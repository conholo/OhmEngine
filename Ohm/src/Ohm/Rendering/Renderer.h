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
		static void DrawMesh(const EditorCamera& camera, const Ref<Mesh>& mesh, const Ref<Material>& material, const TransformComponent& transform);
		static void DrawMesh(const EditorCamera& camera, const Ref<Mesh>& mesh);
		static void DrawFullScreenQuad();
		static void DrawUnitCube();
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
		static void UploadCameraUniformData(const EditorCamera& camera, const TransformComponent& transform);

	private:
		static Statistics s_Stats;
	};
}