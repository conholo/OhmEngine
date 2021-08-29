#pragma once

#include "Ohm/Scene/Component.h"
#include "Ohm/Rendering/EditorCamera.h"
#include <entt.hpp>

namespace Ohm
{
	class Entity;

	class Scene
	{
	public:

		Scene(const std::string& name = "Sample Scene")
			: m_SceneName(name) { }

		Entity Create(const std::string& name = "Entity");
		bool Destroy(Entity entity);

		const std::string& GetName() const { return m_SceneName; }

	private:
		std::string m_SceneName;
		entt::registry m_Registry;

		friend class Entity;
		friend class EditorScene;
	};
}