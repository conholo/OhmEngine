#include "ohmpch.h"
#include "Ohm/Scene/Scene.h"

#include "Ohm/Scene/Entity.h"
#include "Ohm/Rendering/Renderer.h"

namespace Ohm
{
	Entity Scene::Create(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };

		entity.AddComponent<TagComponent>(name.empty() ? "Entity" : name);
		entity.AddComponent<TransformComponent>();

		return entity;
	}

	bool Scene::Destroy(Entity entity)
	{
		if (m_Registry.valid(entity))
		{
			m_Registry.destroy(entity);
		}

		return false;
	}
}