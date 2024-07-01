#include "EditorLayer.h"
#include "Panels/Dockspace.h"
#include "Ohm/Rendering/SceneRenderer.h"

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <cmath>

#include "Ohm/Rendering/TextureLibrary.h"

namespace Ohm
{
	EditorLayer::EditorLayer()
		:Layer("Editor Layer")
	{

	}

	EditorLayer::~EditorLayer()
	{

	}

	void EditorLayer::OnAttach()
	{
		Application::GetApplication().GetWindow().ToggleIsMaximized();
		m_EngineGeometryMaterial = CreateRef<Material>("Base Material", ShaderLibrary::Get("PBR"));

		m_Scene = CreateRef<Scene>("Test Scene");
		auto sun = m_Scene->CreateEntity("Sun");
		sun.AddComponent<PrimitiveRendererComponent>(Primitive::Sphere, "PBR");
		sun.AddComponent<DirectionalLightComponent>();
		sun.GetComponent<DirectionalLightComponent>().Intensity = 5.0f;
		sun.GetComponent<TransformComponent>().Scale = {1.0f, 1.0f, 1.0f};

		auto envLight = m_Scene->CreateEntity("Env");
		envLight.AddComponent<EnvironmentLightComponent>();
		envLight.GetComponent<EnvironmentLightComponent>().Pipeline->GetSpecification().PipelineType = EnvironmentPipelineType::FromShader; 
		envLight.GetComponent<EnvironmentLightComponent>().EnvironmentMapParams.Inclination = glm::radians(50.0f);

		SceneRenderer::LoadScene(m_Scene);
		SceneRenderer::InitializePipeline();

		m_SceneHierarchyPanel.SetContext(m_Scene);
		m_ViewportPanel.SetFramebuffer(SceneRenderer::GetSceneCompositeFBO());
	}

	void EditorLayer::OnUpdate(float deltaTime)
	{
		SceneRenderer::ValidateResize(m_ViewportPanel.GetViewportSize());
		SceneRenderer::UpdateCamera(deltaTime);
		SceneRenderer::SubmitPipeline();
	}

	void EditorLayer::OnDetach()
	{
		OHM_INFO("On Detach");
		SceneRenderer::UnloadScene();
	}

	void EditorLayer::OnUIRender()
	{
		Dockspace::Begin();
		Dockspace::Draw();
		// Menu Bar
		{
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					ImGui::Separator();
					if (ImGui::MenuItem("Quit"))
						Application::Close();
					ImGui::Separator();

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Create"))
				{
					ImGui::Separator();
					if (ImGui::BeginMenu("3D-Primitives"))
					{
						ImGui::Separator();
						
						if (ImGui::MenuItem("Cube"))
						{
							Entity cube = m_Scene->CreateEntity("Cube");
							cube.AddComponent<MeshRendererComponent>(m_EngineGeometryMaterial->Clone("Cube Base Material"), MeshFactory::Create(Primitive::Cube));
							m_SceneHierarchyPanel.SetSelectedEntity(cube);
						}
						ImGui::Separator();

						if (ImGui::MenuItem("Sphere"))
						{
							Entity sphere = m_Scene->CreateEntity("Sphere");
							sphere.AddComponent<MeshRendererComponent>(m_EngineGeometryMaterial->Clone("Sphere Base Material"), MeshFactory::Create(Primitive::Sphere));
							m_SceneHierarchyPanel.SetSelectedEntity(sphere);
						}
						ImGui::Separator();

						if (ImGui::MenuItem("Quad"))
						{
							Entity quad = m_Scene->CreateEntity("Quad");
							quad.AddComponent<MeshRendererComponent>(m_EngineGeometryMaterial->Clone("Quad Base Material"), MeshFactory::Create(Primitive::Quad));
							m_SceneHierarchyPanel.SetSelectedEntity(quad);
						}
						ImGui::Separator();

						if (ImGui::MenuItem("Plane"))
						{
							Entity plane = m_Scene->CreateEntity("Plane");
							plane.AddComponent<MeshRendererComponent>(m_EngineGeometryMaterial->Clone("Plane Base Material"), MeshFactory::Create(Primitive::Plane));
							m_SceneHierarchyPanel.SetSelectedEntity(plane);
						}

						ImGui::Separator();
						ImGui::EndMenu();
					}

					ImGui::Separator();
					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Save/Load"))
				{
					ImGui::Separator();

					if (ImGui::MenuItem("Save Scene"))
					{
						SceneSerializer serializer(m_Scene);
						std::string filePath = "assets/scenes/TestScene.scene";
						serializer.Serialize(filePath);
					}

					ImGui::Separator();

					if (ImGui::MenuItem("Load Scene"))
					{
						SceneSerializer serializer(m_Scene);
						std::string filePath = "assets/scenes/TestScene.scene";
						serializer.Deserialize(filePath);
					}

					ImGui::Separator();
					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}
		}

		m_SceneHierarchyPanel.Draw();
		// Console
		m_ConsolePanel.Draw("Console");
		// Statistics
		{
			ImGui::Begin("Statistics");

			Renderer::Statistics stats = Renderer::GetStats();

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Text("Vertex Count: %d", stats.VertexCount);
			ImGui::Text("Triangle Count: %d", stats.TriangleCount);
			ImGui::End();
		}

		// Scene Drawer 
		{
			ImGui::Begin("Scene Renderer Parameters");
			SceneRenderer::DrawSceneRendererUI(m_ViewportPanel.GetViewportSize());
			ImGui::End();
			SceneRenderer::DrawTextureViewerUI();
		}

		m_ViewportPanel.Draw();
		Dockspace::End();
	}

	void EditorLayer::OnEvent(Event& event)
	{
		SceneRenderer::OnEvent(event);
	}
}
