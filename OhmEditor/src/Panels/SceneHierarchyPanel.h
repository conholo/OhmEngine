#pragma once

#include "Ohm.h"
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
		void RegisterEntityMaterialProperties(Entity entity);
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);
		void DrawMeshSelection(Entity entity, MeshRendererComponent& meshRenderer);

	private:
		Entity m_SelectedEntity;
		Ref<Scene> m_Scene;


		// TODO:: All of this stuff needs to move into a class dedicated to the material inspector.
		std::set<uint32_t> m_RegisteredMaterialPropertyEntites;
		std::unordered_map<uint32_t, std::unordered_map<std::string, Ref<UI::UIFloat>>>		m_MaterialFloatProperties;
		std::unordered_map<uint32_t, std::unordered_map<std::string, Ref<UI::UIInt>>>		m_MaterialIntProperties;
		std::unordered_map<uint32_t, std::unordered_map<std::string, Ref<UI::UIVector2>>>	m_MaterialVec2Properties;
		std::unordered_map<uint32_t, std::unordered_map<std::string, Ref<UI::UIVector3>>>	m_MaterialVec3Properties;
		std::unordered_map<uint32_t, std::unordered_map<std::string, Ref<UI::UIVector4>>>	m_MaterialVec4Properties;
		std::unordered_map<uint32_t, std::unordered_map<std::string, Ref<UI::UIColor>>>		m_MaterialColorProperties;
		std::unordered_map<uint32_t, std::unordered_map<std::string, Ref<UI::UITexture2D>>>	m_MaterialTextureProperties;

		std::unordered_map<uint32_t, std::unordered_map<std::string, bool>>				m_PrimitiveSelectionOptions;
		std::unordered_map<uint32_t, std::string>										m_SelectedPrimitive;
		std::unordered_map<uint32_t, std::unordered_map<std::string, bool>>				m_MaterialSelectionOptions;
		std::unordered_map<uint32_t, std::string>										m_SelectedMaterial;
	};
}