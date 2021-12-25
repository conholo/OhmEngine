#include "ohmpch.h"
#include "Ohm/Scene/Scene.h"

#include "Ohm/Scene/Entity.h"
#include "Ohm/Rendering/Renderer.h"


namespace Ohm
{
	Scene::Scene(const std::string& name)
		: m_SceneName(name)
	{
		m_SceneLightingBuffer = CreateRef<UniformBuffer>(sizeof(Scene::LightingData), 1);
	}

	Entity Scene::Create(const std::string& name)
	{
		entt::entity id = m_Registry.create();
		Entity entity = { id, this };

		entity.AddComponent<TagComponent>(name.empty() ? "Entity" : name);
		entity.AddComponent<TransformComponent>();

		m_SceneMap.emplace(id, entity);

		return entity;
	}

	bool Scene::Destroy(Entity entity)
	{
		if (m_Registry.valid(entity))
		{
			m_Registry.destroy(entity);
			return true;
		}

		return false;
	}

	Entity& Scene::GetEntityFromSceneMap(entt::entity id)
	{
		return m_SceneMap[id];
	}

	Entity& Scene::GetSunLight()
	{
		return m_SceneMap[(entt::entity)m_SunLightHandle];
	}

	void Scene::SetSceneLightingData(const EditorCamera& camera)
	{
		Entity& sunLight = GetEntityFromSceneMap((entt::entity)m_SunLightHandle);

		LightComponent& lightComponent = sunLight.GetComponent<LightComponent>();
		TransformComponent& lightTransform = sunLight.GetComponent<TransformComponent>();

		m_LightingData.LightColor = lightComponent.Color;
		m_LightingData.LightIntensity = lightComponent.Intensity;
		m_LightingData.LightPosition = lightTransform.Translation;

		m_SceneLightingBuffer->SetData(&m_LightingData, sizeof(Scene::LightingData));
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(false);
	}

	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& tagComponent)
	{

	}

	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& tranformComponent)
	{

	}

	template<>
	void Scene::OnComponentAdded<MeshRendererComponent>(Entity entity, MeshRendererComponent& meshRendererComponent)
	{

	}

	template<>
	void Scene::OnComponentAdded<LightComponent>(Entity entity, LightComponent& lightComponent)
	{
		// TODO:: Assert that there's only one sun.
		if (lightComponent.Type == LightType::Sun)
			m_SunLightHandle = (uint32_t)entity;
	}
}