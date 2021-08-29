#include "ohmpch.h"
#include "Ohm/Scene/Entity.h"

namespace Ohm
{

	Entity::Entity(entt::entity handle, Scene* scene)
		:m_EntityHandle(handle), m_Scene(scene)
	{
	}

}