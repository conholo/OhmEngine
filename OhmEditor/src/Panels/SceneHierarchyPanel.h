#pragma once

#include "Panels/MaterialInspector.h"
#include "Ohm/Scene/Entity.h"

namespace Ohm
{
	namespace UI
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

			MaterialInspector& GetMaterialInspector(Entity E, uint32_t MaterialIndex = 0) ;
			void RegisterEntityMaterialProperties(Entity Entity);

		private:
			void DrawEntityNode(Entity entity);
			void DrawComponents(Entity entity);
			void DrawPrimitiveMeshSelection(PrimitiveRendererComponent& PrimitiveRenderer) const;

		private:
			Entity m_SelectedEntity;
			Ref<Scene> m_Scene;
			std::unordered_map<UUID, std::vector<Ref<MaterialInspector>>> m_RegisteredMaterialInspectors;
			std::string TextureToCubeFilePath;
			std::string TextureToCubeFileName = "2DTextureToCube.png";
		};

	}
}
