#pragma once

#include "Ohm/Scene/Component.h"
#include "Ohm/Rendering/EditorCamera.h"

namespace Ohm
{
	class Renderer
	{
	public:
		static void Initialize();
		static void DrawMesh(const EditorCamera& camera, const MeshRendererComponent& meshRenderer, const TransformComponent& transform);
		static void Shutdown();
	};
}