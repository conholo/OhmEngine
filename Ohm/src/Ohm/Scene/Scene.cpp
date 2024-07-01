#include "ohmpch.h"
#include "Ohm/Scene/Scene.h"

#include "Ohm/Scene/Entity.h"
#include "Ohm/Rendering/Renderer.h"

namespace Ohm
{
	Scene::Scene(const std::string& name)
		: m_SceneName(name)
	{
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
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

	void Scene::UpdateLightingEnvironment(const EditorCamera& camera)
	{
		Entity EnvironmentLight = GetEnvironmentLight();
		const EnvironmentLightComponent& EnvLightComponent = EnvironmentLight.GetComponent<EnvironmentLightComponent>();

		if(EnvLightComponent.Pipeline->GetSpecification().PipelineType != EnvironmentPipelineType::FromShader) return;

		const float Azimuth = EnvLightComponent.EnvironmentMapParams.Azimuth;
		const float Elevation = EnvLightComponent.EnvironmentMapParams.Inclination;
			
		Entity DirectionalLightEntity = GetDirectionalLight();
		DirectionalLightComponent& DirLightComponent = DirectionalLightEntity.GetComponent<DirectionalLightComponent>();
		const glm::vec3 DirectionToSun = normalize(glm::vec3
			(
				sin(Azimuth) * cos(Elevation),
				sin(Elevation),
				cos(Elevation) * cos(Azimuth)
			)
		);
		DirLightComponent.LightDirection = -DirectionToSun;
	}

	Entity Scene::CreateEntityWithUUID(UUID UUID, const std::string& Name)
	{
		Entity Entity = { m_Registry.create(), this };
		Entity.AddComponent<IDComponent>(UUID);
		Entity.AddComponent<TransformComponent>();
		auto& tag = Entity.AddComponent<TagComponent>();
		tag.Tag = Name.empty() ? "Entity" : Name;
		return Entity;
	}
	
	Entity Scene::GetDirectionalLight()
	{
		const auto View = m_Registry.view<DirectionalLightComponent>();
		for (const auto entity : View)
			return Entity{entity, this};
		return {};
	}

	Entity Scene::GetEnvironmentLight()
	{
		const auto View = m_Registry.view<EnvironmentLightComponent>();
		for (const auto entity : View)
			return Entity{entity, this};
		return {};
	}
	
	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(sizeof(T) == 0);
	}

	template<>
	void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<PrimitiveRendererComponent>(Entity entity, PrimitiveRendererComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<MeshRendererComponent>(Entity entity, MeshRendererComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<DirectionalLightComponent>(Entity entity, DirectionalLightComponent& lightComponent)
	{
		m_DirectionalLightEntityID = entity;  
	}
	
	template<>
	void Scene::OnComponentAdded<EnvironmentLightComponent>(Entity entity, EnvironmentLightComponent& component)
	{
		m_EnvironmentLightEntityID = entity;  
	}

}