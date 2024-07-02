#pragma once

#include "CloudsUI.h"
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

			MaterialInspector& GetMaterialInspector(Entity e, uint32_t materialIndex = 0) ;
			void TryRegisterEntityMaterialProperties(Entity entity);
			void TryRegisterEntityVolumetricCloudsUI(Entity entity);

		private:
			void DrawEntityNode(Entity entity);
			void DrawComponents(Entity entity);
			void DrawPrimitiveMeshSelection(PrimitiveRendererComponent& PrimitiveRenderer) const;

		private:
			Entity m_SelectedEntity;
			Ref<Scene> m_Scene;
			std::unordered_map<UUID, std::vector<Ref<MaterialInspector>>> m_RegisteredMaterialInspectors;
			std::unordered_map<UUID, Ref<CloudsUI>> m_RegisteredCloudInspectors;
			std::string TextureToCubeFilePath;
			std::string TextureToCubeFileName = "2DTextureToCube.png";
		};

	}
}
