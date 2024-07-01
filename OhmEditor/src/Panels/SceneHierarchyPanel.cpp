#include "SceneHierarchyPanel.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <entt.hpp>
#include <imgui/imfilebrowser.h>

#include "Ohm/UI/UIDrawerHelpers.h"

namespace Ohm
{
    namespace UI
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

		MaterialInspector& SceneHierarchyPanel::GetMaterialInspector(Entity E, uint32_t MaterialIndex)
		{
        	UUID ID = E.GetComponent<IDComponent>().ID;
        	ASSERT(m_RegisteredMaterialInspectors.find(ID) != m_RegisteredMaterialInspectors.end(), "Invalid UUID when indexing Scene Hierarchy panel for Material Inspector.");
        	const std::vector<Ref<MaterialInspector>> AvailableMaterialInspectors = m_RegisteredMaterialInspectors[ID];
        	ASSERT(!AvailableMaterialInspectors.empty() && MaterialIndex <= AvailableMaterialInspectors.size() - 1, "Invalid index of material inspector for entity with UUID: {}", ID);
        	return *AvailableMaterialInspectors[MaterialIndex];
		}

		void SceneHierarchyPanel::RegisterEntityMaterialProperties(Entity Entity)
		{
        	if(m_RegisteredMaterialInspectors.find(Entity.GetComponent<IDComponent>().ID) != m_RegisteredMaterialInspectors.end()) return;
        	
			const bool HasMeshRenderer = Entity.HasComponent<MeshRendererComponent>();
			const bool HasPrimitiveMeshRenderer = Entity.HasComponent<PrimitiveRendererComponent>();

			if (!HasMeshRenderer && !HasPrimitiveMeshRenderer)
				return;
        	
			if(HasPrimitiveMeshRenderer)
			{
				auto& PrimitiveMeshRenderer = Entity.GetComponent<PrimitiveRendererComponent>();
				if(PrimitiveMeshRenderer.PrimitiveType != Primitive::None && PrimitiveMeshRenderer.MaterialInstance != nullptr)
					m_RegisteredMaterialInspectors[Entity.GetComponent<IDComponent>().ID].push_back(CreateRef<MaterialInspector>(PrimitiveMeshRenderer.MaterialInstance));
			}
		}

		void SceneHierarchyPanel::Draw()
		{
			ImGui::Begin("Scene Hierarchy");
			{
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

				if (ImGui::BeginPopupContextWindow(0, 1))
				{
					if (ImGui::MenuItem("Create Entity"))
					{
						const Entity Entity = m_Scene->CreateEntity("Entity");
						m_SelectedEntity = Entity;
					}

					ImGui::EndPopup();
				}
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

			const bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());

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
				bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(9817238), flags, tag.c_str());

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
						OHM_WARN("Only one mesh renderer component is allowed per entity.");

					if (m_SelectedEntity.HasComponent<PrimitiveRendererComponent>())
						OHM_WARN("An entity may only have a MeshRenderer or a PrimitiveMeshRenderer, not both.");
				}

				if (ImGui::MenuItem("Primitive Mesh Renderer"))
				{
					if (!m_SelectedEntity.HasComponent<PrimitiveRendererComponent>())
						m_SelectedEntity.AddComponent<PrimitiveRendererComponent>();
					else
						OHM_WARN("Only one primitive mesh renderer component is allowed per entity.");

					if (m_SelectedEntity.HasComponent<MeshRendererComponent>())
						OHM_WARN("An entity may only have a MeshRenderer or a PrimitiveMeshRenderer, not both.");
				}

				if (ImGui::MenuItem("Directional Light"))
				{
					if (!m_SelectedEntity.HasComponent<DirectionalLightComponent>())
						m_SelectedEntity.AddComponent<DirectionalLightComponent>();
					else
						OHM_WARN("Only one light component is allowed per entity.");
				}

				ImGui::EndPopup();
			}

			ImGui::PopItemWidth();

			//-------------------------TRANSFORM-------------------------//
			// Transform
			auto DrawTransformFn = [](auto& component, Entity entity)
			{
				UI::DrawVector3Field("Position", component.Translation);
				UI::DrawVector3Field("Rotation", component.RotationDegrees);
				UI::DrawVector3Field("Scale", component.Scale, 1.0f);
			};
			auto cleanUpTransform = [](auto& component, Entity entity) {};

			DrawComponent<TransformComponent>("Transform", entity, DrawTransformFn, cleanUpTransform);
			//-------------------------TRANSFORM-------------------------//


			//-------------------------PRIMITIVE RENDERER-------------------------//
			auto DrawPrimitiveMeshRendererFn = [this](auto& Component, Entity Entity)
			{
				if(Component.MaterialInstance == nullptr)
					if (ImGui::Button("Add Default Material"))
					{
						Component.MaterialInstance = CreateRef<Material>("Default Engine Material", ShaderLibrary::Get("PBR"));
						TextureUniform albedo{TextureLibrary::Get2D("White Texture")->GetID(), 0, 0 };
						TextureUniform normal{TextureLibrary::Get2D("White Texture")->GetID(), 1, 0 };
						TextureUniform metal{TextureLibrary::Get2D("White Texture")->GetID(), 2, 0 };
						TextureUniform roughness{TextureLibrary::Get2D("White Texture")->GetID(), 3, 0 };

						Component.MaterialInstance->Set("sampler_AlbedoTexture", 	albedo);
						Component.MaterialInstance->Set("sampler_NormalTexture", 	normal);
						Component.MaterialInstance->Set("sampler_MetalnessTexture", metal);
						Component.MaterialInstance->Set("sampler_RoughnessTexture", roughness);
					}

				constexpr ImGuiTreeNodeFlags TreeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;
				if (ImGui::TreeNodeEx("Primitive Type Selector", TreeNodeFlags))
				{
					DrawPrimitiveMeshSelection(Component);
					ImGui::TreePop();
				}
				if(Component.PrimitiveType == Primitive::None) return;
				
				if (ImGui::TreeNodeEx("Material Properties", TreeNodeFlags))
				{
					if(m_RegisteredMaterialInspectors.find(Entity.GetComponent<IDComponent>().ID) == m_RegisteredMaterialInspectors.end())
						RegisterEntityMaterialProperties(Entity);
					m_RegisteredMaterialInspectors[Entity.GetComponent<IDComponent>().ID][0]->Draw();
					ImGui::TreePop();
				}
			};
			auto CleanUpPrimitiveMeshRendererFn = [this](auto& component, Entity entity)
			{
			};

			DrawComponent<PrimitiveRendererComponent>("Primitive Mesh Renderer", entity, DrawPrimitiveMeshRendererFn, CleanUpPrimitiveMeshRendererFn);
			//-------------------------PRIMITIVE RENDERER-------------------------//


        	//-------------------------MESH RENDERER-------------------------//
        	auto DrawMeshRendererFn = [this](auto& Component, Entity Entity)
        	{
        		constexpr ImGuiTreeNodeFlags TreeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;
        		if (ImGui::TreeNodeEx("Material Properties", TreeNodeFlags))
        		{
        			for(int i = 0; i < m_RegisteredMaterialInspectors[Entity.GetComponent<IDComponent>().ID].size(); i++)
        			{
        				if(ImGui::TreeNodeEx(m_RegisteredMaterialInspectors[Entity.GetComponent<IDComponent>().ID][i]->GetMaterialName().c_str()))
        				{
        					m_RegisteredMaterialInspectors[Entity.GetComponent<IDComponent>().ID][i]->Draw();
        					ImGui::TreePop();
        				}
        			}
        			ImGui::TreePop();
        		}
        	};
        	auto CleanUpMeshRendererFn = [this](auto& component, Entity entity)
        	{
        	};

        	DrawComponent<MeshRendererComponent>("Mesh Renderer", entity, DrawMeshRendererFn, CleanUpMeshRendererFn);
        	//-------------------------MESH RENDERER-------------------------//


			//-------------------------DIRECTIONAL LIGHT-------------------------//
			auto DrawLightFn = [](auto& Component, Entity entity)
			{
				UIVector3::Draw("Radiance", &Component.Radiance);
				UIVector3::Draw("Direction", &Component.LightDirection);
				UIFloat::Draw("Intensity", &Component.Intensity);
				UIFloat::Draw("Shadow Amount", &Component.ShadowAmount);
			};
			auto CleanUpLightFn = [](auto& component, Entity entity) {};

			DrawComponent<DirectionalLightComponent>("Directional Light", entity, DrawLightFn, CleanUpLightFn);
			//-------------------------DIRECTIONAL LIGHT-------------------------//

        	//-------------------------ENVIRONMENT LIGHT-------------------------//
        	auto DrawEnvLightFn = [this](auto& Component, Entity entity)
        	{
                const std::string CurrentPipelineTypeName = EnvironmentUtils::PipelineTypeToString(Component.Pipeline->GetSpecification().PipelineType);

        		if (ImGui::BeginCombo("Pipeline Selection Type", CurrentPipelineTypeName.c_str()))
        		{
        			for (const auto [Name, Type] : EnvironmentUtils::GetTypeMap())
        			{
        				const bool Selected = CurrentPipelineTypeName == Name;
        				if (ImGui::Selectable(Name.c_str(), Selected))
        				{
        					Component.Pipeline->GetSpecification().PipelineType = Type;
        					Component.NeedsUpdate = true;
        				}
        			}

        			ImGui::EndCombo();
        		}

        		if(Component.Pipeline->GetSpecification().PipelineType == EnvironmentPipelineType::BlackCube)
        			return;

        		if(Component.Pipeline->GetSpecification().PipelineType == EnvironmentPipelineType::FromShader)
        		{
        			if(ImGui::CollapsingHeader("Preetham Sky Settings", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed))
        			{
        				char label[150];

        				Component.NeedsUpdate |= UIFloat::DrawSlider("Turbidity", &Component.EnvironmentMapParams.Turbidity, 2.0, 20.0);
        				ImGui::SameLine();
        				sprintf(label, "Air Turbidity, value in range [%.1f, %.1f]", 2.0, 20.0);
        				HelpMarker(label);

        				Component.NeedsUpdate |= UIFloat::DrawAngle("Azimuth", &Component.EnvironmentMapParams.Azimuth, 0.0f, 360.0f);
        				ImGui::SameLine();
        				HelpMarker("Sun azimuth at view point in degrees, value in range [0, 360]");

        				Component.NeedsUpdate |= UIFloat::DrawAngle("Elevation", &Component.EnvironmentMapParams.Inclination, -4.2f, 90.0f);
        				ImGui::SameLine();
        				sprintf(label, "Sun elevation at view point in degrees, value in range [%.1f, %.1f]", -4.2f, 90.0f);
        				HelpMarker(label);

        				Component.NeedsUpdate |= ImGui::Button("Update");
        			}
        		}

        		if(Component.Pipeline->GetSpecification().PipelineType == EnvironmentPipelineType::FromFile)
        		{
        			ImGui::FileBrowser Browser;
        			Browser.SetTitle("Select Cubemap File");
        			Browser.SetTypeFilters({ ".png", ".jpg" });
        			const std::filesystem::path AssetPath = "../assets/textures/";
        			Browser.SetPwd(AssetPath);

        			ImGui::SameLine();
        			ImGui::Text("file");
        			ImGui::SameLine();
        			HelpMarker("2D file to be converted to a cubemap");
        			Browser.Display();
        			if (Browser.HasSelected())
        			{
        				TextureToCubeFilePath = Browser.GetSelected().string();
        				TextureToCubeFileName = TextureToCubeFilePath.substr(TextureToCubeFilePath.find_last_of("\\") + 1);
        				Component.Pipeline->GetSpecification().FromFileFilePath = TextureToCubeFilePath;
        				Browser.ClearSelected();
        			}

        			if (ImGui::Button(TextureToCubeFileName.c_str(), ImVec2(ImGui::CalcItemWidth(), 20)))
        				Browser.Open();
        		}
        		
        		UIVector3::Draw("LODs", &Component.EnvironmentMapSampleLODs);
        		UIVector3::Draw("Intensities", &Component.EnvironmentMapSampleIntensities);
        	};
        	
        	auto CleanUpEnvLightFn = [](auto& component, Entity entity) {};

        	DrawComponent<EnvironmentLightComponent>("Environment Light", entity, DrawEnvLightFn, CleanUpEnvLightFn);
        	//-------------------------ENVIRONMENT LIGHT-------------------------//
		}

		void SceneHierarchyPanel::DrawPrimitiveMeshSelection(PrimitiveRendererComponent& PrimitiveRenderer) const
		{
        	const std::string PrimitiveName = MeshFactory::MeshPrimitiveToString(PrimitiveRenderer.PrimitiveType);

        	if (ImGui::BeginCombo("Mesh Selection", PrimitiveName.c_str()))
        	{
        		for (const auto AvailablePrimitiveName : {"None", "Quad", "Cube", "Sphere"})
        		{
	                const bool Selected = AvailablePrimitiveName == PrimitiveName;
        			if (ImGui::Selectable(AvailablePrimitiveName, Selected))
        				PrimitiveRenderer.PrimitiveType = MeshFactory::StringToPrimitiveType(AvailablePrimitiveName);
        		}

        		ImGui::EndCombo();
        	}
		}
    }
}
