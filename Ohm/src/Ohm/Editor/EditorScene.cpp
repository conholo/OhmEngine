#include "ohmpch.h"
#include "Ohm/Editor/EditorScene.h"
#include "Ohm/Rendering/Renderer.h"


namespace Ohm
{
	Ref<Scene> EditorScene::s_ActiveScene = nullptr;

	void EditorScene::LoadScene(const Ref<Scene>& runtimeScene)
	{
		s_ActiveScene = runtimeScene;
	}

	void EditorScene::UnloadScene()
	{
		s_ActiveScene = nullptr;
	}

	void EditorScene::RenderScene(const EditorCamera& camera, Entity directionalLight)
	{
		auto group = s_ActiveScene->m_Registry.group<TransformComponent, MeshRendererComponent>();

		auto lightTransform = s_ActiveScene->m_Registry.get<TransformComponent>(directionalLight);

		for (auto entity : group)
		{
			auto [transform, meshRenderer] = group.get<TransformComponent, MeshRendererComponent>(entity);

			Renderer::DrawMesh(camera, meshRenderer, transform, lightTransform);
		}
	}
}