#include "Panels/SceneHierarchyPanel.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <entt.hpp>
#include <algorithm>

#define _CRT_SECURE_NO_WARNINGS

namespace Ohm
{
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene)
	{
		SetContext(scene);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& scene)
	{
		m_Scene = scene;
		m_SelectedEntity = {};
	}

	void SceneHierarchyPanel::Draw()
	{
		ImGui::Begin("Scene Hierarchy");

		m_Scene->m_Registry.each([&](auto entityID)
			{
				Entity entity{ entityID, m_Scene.get() };

				bool registered = m_RegisteredMaterialPropertyEntites.find((uint32_t)entityID) != m_RegisteredMaterialPropertyEntites.end();

				if (entity.HasComponent<MeshRendererComponent>() && !registered)
				{
					m_RegisteredMaterialPropertyEntites.insert((uint32_t)entityID);
					auto& meshrenderer = entity.GetComponent<MeshRendererComponent>();
					auto& uniforms = meshrenderer.MaterialInstance->GetShader()->GetUniforms();

					for (auto [name, uniform] : uniforms)
					{
						size_t offset = name.find_first_of('_', 0);
						std::string strippedName = name.substr(offset + 1, strlen(name.c_str()));

						std::string strippedToLower = std::string(strippedName);
						std::transform(strippedToLower.begin(), strippedToLower.end(), strippedToLower.begin(), ::tolower);
						bool colorFound = strippedToLower.find("color") != std::string::npos;

						UIPropertyType propertyType = UIPropertyTypeFromShaderDataType(uniform.GetType(), colorFound);

						switch (propertyType)
						{
						case UIPropertyType::Float:
						{
							float* value = meshrenderer.MaterialInstance->Get<float>(name);
							m_MaterialFloatProperties[(uint32_t)entity][name] = CreateRef<UIFloat>(strippedName, value);
							break;
						}
						case UIPropertyType::Int:
						{
							int* value = meshrenderer.MaterialInstance->Get<int>(name);
							m_MaterialIntProperties[(uint32_t)entity][name] = CreateRef<UIInt>(strippedName, value);
							break;
						}
						case UIPropertyType::Vec2:
						{
							glm::vec2* value = meshrenderer.MaterialInstance->Get<glm::vec2>(name);
							m_MaterialVec2Properties[(uint32_t)entity][name] = CreateRef<UIVector2>(strippedName, value);
							break;
						}
						case UIPropertyType::Vec3:
						{
							glm::vec3* value = meshrenderer.MaterialInstance->Get<glm::vec3>(name);
							m_MaterialVec3Properties[(uint32_t)entity][name] = CreateRef<UIVector3>(strippedName, value);
							break;
						}
						case UIPropertyType::Vec4:
						{
							glm::vec4* value = meshrenderer.MaterialInstance->Get<glm::vec4>(name);
							m_MaterialVec4Properties[(uint32_t)entity][name] = CreateRef<UIVector4>(strippedName, value);
							break;
						}
						case UIPropertyType::Color:
						{
							glm::vec4* value = meshrenderer.MaterialInstance->Get<glm::vec4>(name);
							m_MaterialColorProperties[entity][name] = CreateRef<UIColor>(strippedName, value);
							break;
						}
						}
					}
				}
				DrawEntityNode(entity);
			});

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
		{
			m_SelectedEntity = {};
		}

		if (ImGui::BeginPopupContextWindow(0, 1, false))
		{
			if (ImGui::MenuItem("Create Entity"))
				m_Scene->Create("Entity");

			ImGui::EndPopup();
		}
		ImGui::End();


		ImGui::Begin("Properties");
		if (m_SelectedEntity)
			DrawComponents(m_SelectedEntity);
		ImGui::End();

	}

	static void DrawVector3Field(const std::string& label, glm::vec3& value, float resetValue = 0.0)
	{
		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, 100.0f);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		if (ImGui::Button("X", buttonSize))
			value.x = resetValue;

		ImGui::SameLine();
		ImGui::DragFloat("##X", &value.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		if (ImGui::Button("Y", buttonSize))
			value.y = resetValue;

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &value.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		if (ImGui::Button("Z", buttonSize))
			value.z = resetValue;

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &value.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();


		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags = ((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());

		if (ImGui::IsItemClicked())
		{
			m_SelectedEntity = entity;
		}
		
		bool entityDeleted = false;

		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		if (opened)
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			bool opened = ImGui::TreeNodeEx((void*)9817238, flags, tag.c_str());

			if (opened)
				ImGui::TreePop();

			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			m_Scene->Destroy(entity);
			if (m_SelectedEntity == entity)
				m_SelectedEntity = {};
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>();
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			std::strncpy(buffer, tag.Tag.c_str(), sizeof(buffer));

			if (ImGui::InputText("##Text", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
		}

		if (entity.HasComponent<TransformComponent>())
		{
			auto& transform = entity.GetComponent<TransformComponent>();
			DrawVector3Field("Position", transform.Translation);
			auto& rotationDegrees = glm::degrees(transform.Rotation);
			DrawVector3Field("Rotation", rotationDegrees);
			transform.Rotation = glm::radians(rotationDegrees);
			DrawVector3Field("Scale", transform.Scale, 1.0f);
		}

		if (entity.HasComponent<MeshRendererComponent>())
		{
			auto& materialInstance = entity.GetComponent<MeshRendererComponent>().MaterialInstance;

			if (ImGui::Button("Dump Shader Data"))
				materialInstance->GetShader()->DumpShaderData();

			bool receiveShadows = materialInstance->ReceivesShadows();
			bool castShadows = materialInstance->CastsShadows();
			ImGui::Checkbox("Receive Shadows", &receiveShadows);
			ImGui::Checkbox("Cast Shadows", &castShadows);

			materialInstance->SetCastsShadows(castShadows);
			materialInstance->SetReceivesShadows(receiveShadows);

			if(m_MaterialFloatProperties.find(entity) != m_MaterialFloatProperties.end())
				for (auto [uniformName, uiFloat] : m_MaterialFloatProperties[entity])
					uiFloat->Draw();
			if (m_MaterialIntProperties.find(entity) != m_MaterialIntProperties.end())
				for (auto [uniformName, uiInt] : m_MaterialIntProperties[entity])
					uiInt->Draw();
			if (m_MaterialVec2Properties.find(entity) != m_MaterialVec2Properties.end())
				for (auto [uniformName, uiVec2] : m_MaterialVec2Properties[entity])
					uiVec2->Draw();
			if (m_MaterialVec3Properties.find(entity) != m_MaterialVec3Properties.end())
				for (auto [uniformName, uiVec3] : m_MaterialVec3Properties[entity])
					uiVec3->Draw();
			if (m_MaterialVec4Properties.find(entity) != m_MaterialVec4Properties.end())
				for (auto [uniformName, uiVec4] : m_MaterialVec4Properties[entity])
					uiVec4->Draw();
			if (m_MaterialColorProperties.find(entity) != m_MaterialColorProperties.end())
				for (auto [uniformName, uiColor] : m_MaterialColorProperties[entity])
					uiColor->Draw();
		}

		if (entity.HasComponent<LightComponent>())
		{
			LightComponent& light = entity.GetComponent<LightComponent>();

			ImGui::ColorPicker4("Light Color", &light.Color.x);
			ImGui::DragFloat("Light Intensity", &light.Intensity, 0.01f, 0.0f, 1.0f);
		}
	}
}