#pragma once

#include "Ohm/Scene/Scene.h"
#include "Ohm/Scene/Entity.h"

namespace Ohm
{
	class EditorScene
	{
	public:

		static void LoadScene(const Ref<Scene>& runtimeScene);
		static void UnloadScene();
		static void RenderScene(const EditorCamera& camera, Entity directionalLight);

	private:
		static Ref<Scene> s_ActiveScene;
	};
}