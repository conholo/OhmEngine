#pragma once

#include "Ohm/Scene/Component.h"
#include <entt.hpp>

#include "Ohm/Rendering/EditorCamera.h"

namespace Ohm
{
	class Entity;

	class Scene
	{
	public:

		Scene(const std::string& name = "Sample Scene");

		Entity CreateEntity(const std::string& name = "Entity");
		bool Destroy(Entity entity);

		const std::string& GetName() const { return m_SceneName; }
		void UpdateLightingEnvironment(const EditorCamera& camera);
		Entity GetDirectionalLight();
		Entity GetEnvironmentLight();

		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Components...>();
		}
		entt::registry m_Registry;
	private:
		Entity CreateEntityWithUUID(UUID UUID, const std::string& Name);
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

	private:
		std::unordered_map<entt::entity, Entity> m_SceneMap;

		uint32_t m_DirectionalLightEntityID;
		uint32_t m_EnvironmentLightEntityID;
		std::string m_SceneName;

		friend class Entity;
		friend class SceneRenderer;
		friend class SceneHierarchyPanel;
		friend class SceneSerializer;
	};
}
