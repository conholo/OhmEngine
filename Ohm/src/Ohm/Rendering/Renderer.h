#pragma once

#include "Ohm/Scene/Component.h"
#include "Ohm/Rendering/EditorCamera.h"

namespace Ohm
{
	class Renderer
	{
	public:
		static void Initialize();
		static void BeginScene(const EditorCamera& camera, const MeshRendererComponent& meshRenderer);
		static void UploadModelData(const glm::vec3& position, const glm::vec3& size);
		static void UploadModelData(const glm::mat4& transform);
		static void EndScene();
		static void Shutdown();
	};
}