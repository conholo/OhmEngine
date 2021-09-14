#pragma once

#include "Ohm.h"
#include "Panels/PropertyDrawer.h"
#include <set>

namespace Ohm
{
	struct MaterialPropertyDrawData
	{
		ShaderUniformType Type;
		bool HideInInspector;
	};

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

		std::set<uint32_t> m_RegisteredMaterialPropertyEntites;
		std::unordered_map<uint32_t, std::unordered_map<std::string, Ref<UIFloat>>>		m_MaterialFloatProperties;
		std::unordered_map<uint32_t, std::unordered_map<std::string, Ref<UIInt>>>		m_MaterialIntProperties;
		std::unordered_map<uint32_t, std::unordered_map<std::string, Ref<UIVector2>>>	m_MaterialVec2Properties;
		std::unordered_map<uint32_t, std::unordered_map<std::string, Ref<UIVector3>>>	m_MaterialVec3Properties;
		std::unordered_map<uint32_t, std::unordered_map<std::string, Ref<UIVector4>>>	m_MaterialVec4Properties;
		std::unordered_map<uint32_t, std::unordered_map<std::string, Ref<UIColor>>>		m_MaterialColorProperties;
	};
}