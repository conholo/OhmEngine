#pragma once

#include "Ohm/Rendering/EditorCamera.h"

namespace Ohm
{
	enum class Primitive
	{
		Quad, Cube
	};

	class Renderer
	{
	public:
		static void Initialize();
		static void BeginScene(const EditorCamera& camera, Primitive primitve);
		static void UploadModelData(const glm::vec3& position, const glm::vec3& size);
		static void UploadModelData(const glm::mat4& transform);
		static void EndScene();
		static void Shutdown();
	};
}