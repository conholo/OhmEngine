#pragma once

#include "Ohm/Scene/Component.h"
#include "Ohm/Rendering/EditorCamera.h"
#include "Ohm/Rendering/RenderPass.h"
#include "Ohm/Rendering/Mesh.h"
#include "Ohm/Scene/Scene.h"

namespace Ohm
{
	class Renderer
	{
	public:
		static void Initialize();

		static void UploadGlobalData();
		static void UploadCameraData(const EditorCamera& Camera);
		static void UploadSceneData(const Ref<Scene>& Scene);
		static void UploadPerEntityData(const TransformComponent& transform);

		static void BeginScene(const Ref<Scene>& scene, const EditorCamera& camera);
		static void EndScene();

		static void BeginPass(const Ref<RenderPass>& renderPass);
		static void EndPass(const Ref<RenderPass>& renderPass);

		static void DrawPrimitive(const PrimitiveRendererComponent& primitive);
		static void DrawPrimitive(const PrimitiveRendererComponent& primitive, const Ref<Material>& material);
		static void DrawFullScreenQuad(const Ref<Material>& material);
		static void DrawSkybox(const Ref<Material>& skyboxMaterial);

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
