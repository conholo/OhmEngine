#include "Panels/SceneHierarchyPanel.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <entt.hpp>

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
			m_SelectedEntity = entity;
		
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

			const auto& uniforms = materialInstance->GetShader()->GetUniforms();

			for (const auto& [name, uniform] : uniforms)
			{
				size_t offset = name.find_first_of('_', 0);

				std::string strippedName = name.substr(offset + 1, strlen(name.c_str()));

				switch (uniform.GetType())
				{
					case ShaderDataType::Float:
					{
						float value = materialInstance->GetStaged<float>(name);
						ImGui::DragFloat(strippedName.c_str(), &value, 0.01, 0.0, 1.0);
						materialInstance->StageUniform<float>(name, value);
						break;
					}
					case ShaderDataType::Float2:
					{
						glm::vec2 value = materialInstance->GetStaged<glm::vec2>(name);
						ImGui::DragFloat2(strippedName.c_str(), &value.x, 0.01, 0.0, 0.0);
						materialInstance->StageUniform<glm::vec2>(name, value);
						break;
					}
					case ShaderDataType::Float3:
					{
						glm::vec3 value = materialInstance->GetStaged<glm::vec3>(name);
						ImGui::DragFloat3(strippedName.c_str(), &value.x, 0.01, 0.0, 1.0);
						materialInstance->StageUniform<glm::vec3>(name, value);
						break;
					}
					case ShaderDataType::Float4:
					{
						glm::vec4 value = materialInstance->GetStaged<glm::vec4>(name);
						ImGui::ColorPicker4(strippedName.c_str(), &value.x);
						materialInstance->StageUniform<glm::vec4>(name, value);
						break;
					}
					case ShaderDataType::Int:
					{
						int value = materialInstance->GetStaged<int>(name);
						ImGui::InputInt(strippedName.c_str(), &value);;
						materialInstance->StageUniform<int>(name, value);
						break;
					}
				}
			}
		}
	}
}