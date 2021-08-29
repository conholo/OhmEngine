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

	void EditorScene::RenderScene(const EditorCamera& camera)
	{
		auto view = s_ActiveScene->m_Registry.view<TransformComponent>();

		for (auto entity : view)
		{
			Renderer::BeginScene(camera, Primitive::Quad);

			auto transform = view.get<TransformComponent>(entity);

			Renderer::UploadModelData(transform.Transform());

			Renderer::EndScene();
		}
	}
}