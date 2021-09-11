#pragma once

#include "Ohm.h"

namespace Ohm
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& scene);

		void SetContext(const Ref<Scene>& scene);

		void Draw();

		Entity GetSelectedEntity() const { return m_SelectedEntity; }
		void SetSelectedEntity(const Entity& entity) { m_SelectedEntity = entity; }

	private:
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);

	private:
		Entity m_SelectedEntity;
		Ref<Scene> m_Scene;
	};
}