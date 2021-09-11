#pragma once

#include "Ohm/Scene/Component.h"
#include "Ohm/Rendering/EditorCamera.h"
#include "Ohm/Rendering/UniformBuffer.h"
#include <entt.hpp>

#include <glm/glm.hpp>

namespace Ohm
{
	class Entity;

	class Scene
	{
	public:

		Scene(const std::string& name = "Sample Scene");

		Entity Create(const std::string& name = "Entity");
		bool Destroy(Entity entity);

		const std::string& GetName() const { return m_SceneName; }

		Entity& GetEntityFromSceneMap(entt::entity id);

		void SetSceneLightingData(TransformComponent& lightTransform, LightComponent& light, const EditorCamera& camera);
		
	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

	private:
		std::string m_SceneName;
		entt::registry m_Registry;

		std::unordered_map<entt::entity, Entity> m_SceneMap;
		Ref<UniformBuffer> m_SceneLightingBuffer;

		struct LightingData
		{	
			glm::vec4 LightColor;
			glm::vec3 ViewSpaceLightPosition;
			float _paddingl = 0;
			glm::vec3 WorldSpaceLightPosition;
			float LightIntensity;
		};

		LightingData m_LightingData;

		friend class Entity;
		friend class EditorScene;
		friend class SceneHierarchyPanel;
	};
}