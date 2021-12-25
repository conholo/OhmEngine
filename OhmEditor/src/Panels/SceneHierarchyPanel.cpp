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

	void SceneHierarchyPanel::RegisterEntityMaterialProperties(Entity entity)
	{
		bool hasMeshRenderer = entity.HasComponent<MeshRendererComponent>();

		if (!hasMeshRenderer)
			return;

		auto& meshrenderer = entity.GetComponent<MeshRendererComponent>();

		bool registered = m_RegisteredMaterialPropertyEntites.find((uint32_t)entity) != m_RegisteredMaterialPropertyEntites.end();
		bool isComplete = meshrenderer.IsComplete();


		if (m_PrimitiveSelectionOptions.find((uint32_t)entity) == m_PrimitiveSelectionOptions.end())
		{
			if (isComplete)
			{
				Primitive primitive = meshrenderer.MeshData->GetPrimitiveType();
				std::string primitiveToString = Mesh::PrimitiveToString(primitive);
				m_PrimitiveSelectionOptions[(uint32_t)entity] = { {"None", false}, {"Cube", false}, {"Sphere", false}, {"Quad", false}, {"Plane", false} };
				m_PrimitiveSelectionOptions[(uint32_t)entity][primitiveToString] = true;
				m_SelectedPrimitive[(uint32_t)entity] = primitiveToString;
			}
			else
			{
				// Potentially no mesh applied, initialize it with default None.
				m_PrimitiveSelectionOptions[(uint32_t)entity] = { {"None", true}, {"Cube", false}, {"Sphere", false}, {"Quad", false}, {"Plane", false} };
				m_SelectedPrimitive[(uint32_t)entity] = "None";
			}
		}

		if (isComplete && !registered)
		{
			m_RegisteredMaterialPropertyEntites.insert((uint32_t)entity);
			auto& uniforms = meshrenderer.MaterialInstance->GetShader()->GetUniforms();

			for (auto [name, uniform] : uniforms)
			{
				size_t offset = name.find_first_of('_', 0);
				std::string strippedName = name.substr(offset + 1, strlen(name.c_str()));

				std::string strippedToLower = std::string(strippedName);
				std::transform(strippedToLower.begin(), strippedToLower.end(), strippedToLower.begin(), ::tolower);
				bool colorFound = strippedToLower.find("color") != std::string::npos;

				UI::UIPropertyType propertyType = UI::UIPropertyTypeFromShaderDataType(uniform.GetType(), colorFound);

				switch (propertyType)
				{
				case UI::UIPropertyType::Float:
				{
					float* value = meshrenderer.MaterialInstance->Get<float>(name);
					m_MaterialFloatProperties[(uint32_t)entity][name] = CreateRef<UI::UIFloat>(strippedName, value);
					break;
				}
				case UI::UIPropertyType::Int:
				{
					int* value = meshrenderer.MaterialInstance->Get<int>(name);
					m_MaterialIntProperties[(uint32_t)entity][name] = CreateRef<UI::UIInt>(strippedName, value);
					break;
				}
				case UI::UIPropertyType::Vec2:
				{
					glm::vec2* value = meshrenderer.MaterialInstance->Get<glm::vec2>(name);
					m_MaterialVec2Properties[(uint32_t)entity][name] = CreateRef<UI::UIVector2>(strippedName, value);
					break;
				}
				case UI::UIPropertyType::Vec3:
				{
					glm::vec3* value = meshrenderer.MaterialInstance->Get<glm::vec3>(name);
					m_MaterialVec3Properties[(uint32_t)entity][name] = CreateRef<UI::UIVector3>(strippedName, value);
					break;
				}
				case UI::UIPropertyType::Vec4:
				{
					glm::vec4* value = meshrenderer.MaterialInstance->Get<glm::vec4>(name);
					m_MaterialVec4Properties[(uint32_t)entity][name] = CreateRef<UI::UIVector4>(strippedName, value);
					break;
				}
				case UI::UIPropertyType::Color:
				{
					glm::vec4* value = meshrenderer.MaterialInstance->Get<glm::vec4>(name);
					m_MaterialColorProperties[entity][name] = CreateRef<UI::UIColor>(strippedName, value);
					break;
				}
				case UI::UIPropertyType::Texture:
				{
					TextureUniform* value = meshrenderer.MaterialInstance->Get<TextureUniform>(name);
					m_MaterialTextureProperties[entity][name] = CreateRef<UI::UITexture2D>(strippedName, meshrenderer.MaterialInstance, value, name);
					break;
				}
				}
			}
		}
	}

	void SceneHierarchyPanel::Draw()
	{
		ImGui::Begin("Scene Hierarchy");

		m_Scene->m_Registry.each([&](auto entityID)
			{
				Entity entity{ entityID, m_Scene.get() };

				if (!entity) return;

				RegisterEntityMaterialProperties(entity);
				DrawEntityNode(entity);
			});

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
		{
			m_SelectedEntity = {};
		}

		if (ImGui::BeginPopupContextWindow(0, 1, false))
		{
			if (ImGui::MenuItem("Create Entity"))
			{
				Entity entity = m_Scene->Create("Entity");
				m_SelectedEntity = entity;
			}

			ImGui::EndPopup();
		}
		ImGui::End();


		ImGui::Begin("Properties");
		if (m_SelectedEntity)
			DrawComponents(m_SelectedEntity);
		ImGui::End();

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

	template<typename T, typename DrawFn, typename CleanupFn>
	void DrawComponent(const std::string& name, Entity entity, DrawFn drawFn, CleanupFn cleanupFn)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

		if (!entity.HasComponent<T>()) return;

		auto& component = entity.GetComponent<T>();

		ImVec2 contentRegion = ImGui::GetContentRegionAvail();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImGui::Separator();
		bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
		ImGui::PopStyleVar();

		ImGui::SameLine(contentRegion.x - lineHeight * 0.5f);
		if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
			ImGui::OpenPopup("Component Settings");

		bool removeComponent = false;

		if (ImGui::BeginPopup("Component Settings"))
		{
			if (ImGui::MenuItem("Remove Component"))
				removeComponent = true;

			ImGui::EndPopup();
		}

		if (open)
		{
			drawFn(component, entity);
			ImGui::TreePop();
		}

		if (removeComponent)
		{
			cleanupFn(component, entity);
			entity.RemoveComponent<T>();
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

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("Add Component");

		if (ImGui::BeginPopup("Add Component"))
		{
			if (ImGui::MenuItem("Mesh Renderer"))
			{
				if (!m_SelectedEntity.HasComponent<MeshRendererComponent>())
					m_SelectedEntity.AddComponent<MeshRendererComponent>();
				else
					OHM_CORE_WARN("Only one mesh renderer component is allowed per entity.");
			}

			if (ImGui::MenuItem("Directional Light"))
			{
				if (!m_SelectedEntity.HasComponent<LightComponent>())
					m_SelectedEntity.AddComponent<LightComponent>();
				else
					OHM_CORE_WARN("Only one light component is allowed per entity.");
			}

			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

		//-------------------------TRANFORM-------------------------//
		// Transform
		auto drawTransform = [](auto& component, Entity entity)
		{
			UI::DrawVector3Field("Position", component.Translation);
			auto& rotationDegrees = glm::degrees(component.Rotation);
			UI::DrawVector3Field("Rotation", rotationDegrees);
			component.Rotation = glm::radians(rotationDegrees);
			UI::DrawVector3Field("Scale", component.Scale, 1.0f);
		};
		auto cleanUpTransform = [](auto& component, Entity entity) {};

		DrawComponent<TransformComponent>("Transform", entity, drawTransform, cleanUpTransform);
		//-------------------------TRANFORM-------------------------//


		//-------------------------MESH RENDERER-------------------------//
		auto drawMeshRenderer = [this](auto& component, Entity entity)
		{
			if (!component.IsComplete())
			{
				DrawMeshSelection(entity, component);

				if (component.MaterialInstance == nullptr)
				{
					if (ImGui::Button("Add Default Material"))
						component.MaterialInstance = CreateRef<Material>("Default Material", ShaderLibrary::Get("PBR"));
				}
				return;
			}

			const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

			bool openMeshInspector = ImGui::TreeNodeEx("Mesh Settings", treeNodeFlags);

			if (openMeshInspector)
			{
				DrawMeshSelection(entity, component);
				ImGui::TreePop();
			}

			bool openMaterialInspector = ImGui::TreeNodeEx("Material Settings", treeNodeFlags);

			if (openMaterialInspector)
			{
				auto& materialInstance = component.MaterialInstance;

				if (m_MaterialFloatProperties.find(entity) != m_MaterialFloatProperties.end())
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
				if (m_MaterialTextureProperties.find(entity) != m_MaterialTextureProperties.end())
					for (auto [uniformName, uiTexture2D] : m_MaterialTextureProperties[entity])
						uiTexture2D->Draw();

				if (ImGui::Button("Dump Shader Data"))
					materialInstance->GetShader()->DumpShaderData();

				ImGui::TreePop();
			}
		};
		auto cleanUpMeshRenderer = [this](auto& component, Entity entity)
		{
			m_RegisteredMaterialPropertyEntites.erase((uint32_t)entity);
			m_MaterialFloatProperties.erase((uint32_t)entity);
			m_MaterialIntProperties.erase((uint32_t)entity);
			m_MaterialVec2Properties.erase((uint32_t)entity);
			m_MaterialVec3Properties.erase((uint32_t)entity);
			m_MaterialVec4Properties.erase((uint32_t)entity);
			m_MaterialColorProperties.erase((uint32_t)entity);
			m_SelectedPrimitive.erase((uint32_t)entity);
			m_PrimitiveSelectionOptions.erase((uint32_t)entity);
		};

		DrawComponent<MeshRendererComponent>("Mesh Renderer", entity, drawMeshRenderer, cleanUpMeshRenderer);
		//-------------------------MESH RENDERER-------------------------//


		//-------------------------LIGHT-------------------------//
		auto drawLight = [](auto& component, Entity entity)
		{
			LightComponent& light = entity.GetComponent<LightComponent>();



			ImGui::ColorPicker4("Light Color", &light.Color.x);
			ImGui::DragFloat("Light Intensity", &light.Intensity, 0.01f, 0.0f, 1.0f);
		};
		auto cleanUpLight = [](auto& component, Entity entity) {};

		DrawComponent<LightComponent>("Light", entity, drawLight, cleanUpLight);
		//-------------------------LIGHT-------------------------//
	}

	void SceneHierarchyPanel::DrawMeshSelection(Entity entity, MeshRendererComponent& meshRenderer)
	{
		const char* preview = m_SelectedPrimitive[(uint32_t)entity].c_str();

		if (ImGui::BeginCombo("Mesh Selection", preview))
		{
			for (auto [name, selected] : m_PrimitiveSelectionOptions[(uint32_t)entity])
			{
				if (ImGui::Selectable(name.c_str(), &selected))
				{
					if (name == "Cube")
						meshRenderer.MeshData = Mesh::CreatePrimitive(Primitive::Cube);
					else if (name == "Sphere")
						meshRenderer.MeshData = Mesh::CreatePrimitive(Primitive::Sphere);
					else if (name == "Quad")
						meshRenderer.MeshData = Mesh::CreatePrimitive(Primitive::Quad);
					else if (name == "Plane")
						meshRenderer.MeshData = Mesh::CreatePrimitive(Primitive::Plane);

					m_SelectedPrimitive[(uint32_t)entity] = name.c_str();
				}
			}

			ImGui::EndCombo();
		}
	}
}