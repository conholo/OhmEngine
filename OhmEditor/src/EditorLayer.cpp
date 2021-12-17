#include "EditorLayer.h"

#include <imgui/imgui.h>

#include "Panels/Dockspace.h"

#include "Ohm/Rendering/SceneRenderer.h"
#include <glm/glm.hpp>

#include <cmath>

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
		m_EngineGeometryMaterial = CreateRef<Material>("Base Material", ShaderLibrary::Get("Phong"));
		m_Scene = CreateRef<Scene>("Test Scene");
		m_SceneHierarchyPanel.SetContext(m_Scene);
		SceneRenderer::LoadScene(m_Scene);
		SceneRenderer::InitializePipeline();

		m_ViewportPanel.SetFramebuffer(SceneRenderer::GetMainColorBuffer());
		SceneRenderer::ValidateResize(m_ViewportPanel.GetViewportSize());

		m_DirectionalLight = m_Scene->Create("Directional Light");
		m_DirectionalLight.AddComponent<LightComponent>(LightType::Sun, glm::vec4(1.0f), 1.0f, true);
		m_DirectionalLight.AddComponent<MeshRendererComponent>(m_EngineGeometryMaterial->Clone("Directional Light Debug Material"), Mesh::CreatePrimitive(Primitive::Sphere));
		m_DirectionalLight.GetComponent<TransformComponent>().Translation = glm::vec3(0.0f, 10.0f, 0.0f);
		m_DirectionalLight.GetComponent<TransformComponent>().Scale = glm::vec3(0.25f);

		TextureLibrary::Load("assets/textures/lava.jpg");
		TextureLibrary::Load("assets/textures/uv.png");

		//Entity test = m_Scene->Create("Vertex Test");
		//MeshRendererComponent& meshRenderer = test.AddComponent<MeshRendererComponent>(CreateRef<Material>("Vert Deformation Mat", ShaderLibrary::Get("VertexDeformation")), Mesh::CreatePrimitive(Primitive::Plane));
		//test.GetComponent<TransformComponent>().Scale = glm::vec3(10.0f, 1.0f, 10.0f);

	}

	void EditorLayer::OnUpdate(float deltaTime)
	{
		SceneRenderer::ValidateResize(m_ViewportPanel.GetViewportSize());
		SceneRenderer::UpdateEditorCamera(deltaTime);
		SceneRenderer::SubmitPipeline();
	}

	void EditorLayer::OnDetach()
	{
		OHM_INFO("On Detach");
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
					if (ImGui::BeginMenu("3D-Primtives"))
					{
						ImGui::Separator();
						
						if (ImGui::MenuItem("Cube"))
						{
							Entity cube = m_Scene->Create("Cube");
							cube.AddComponent<MeshRendererComponent>(m_EngineGeometryMaterial->Clone("Cube Base Material"), Mesh::CreatePrimitive(Primitive::Cube));
							m_SceneHierarchyPanel.SetSelectedEntity(cube);
						}
						ImGui::Separator();

						if (ImGui::MenuItem("Sphere"))
						{
							Entity sphere = m_Scene->Create("Sphere");
							sphere.AddComponent<MeshRendererComponent>(m_EngineGeometryMaterial->Clone("Sphere Base Material"), Mesh::CreatePrimitive(Primitive::Sphere));
							m_SceneHierarchyPanel.SetSelectedEntity(sphere);
						}
						ImGui::Separator();

						if (ImGui::MenuItem("Quad"))
						{
							Entity quad = m_Scene->Create("Quad");
							quad.AddComponent<MeshRendererComponent>(m_EngineGeometryMaterial->Clone("Quad Base Material"), Mesh::CreatePrimitive(Primitive::Quad));
							m_SceneHierarchyPanel.SetSelectedEntity(quad);
						}
						ImGui::Separator();

						if (ImGui::MenuItem("Plane"))
						{
							Entity plane = m_Scene->Create("Plane");
							plane.AddComponent<MeshRendererComponent>(m_EngineGeometryMaterial->Clone("Plane Base Material"), Mesh::CreatePrimitive(Primitive::Plane));
							m_SceneHierarchyPanel.SetSelectedEntity(plane);
						}

						ImGui::Separator();
						ImGui::EndMenu();
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
		}

		m_ViewportPanel.Draw();
		Dockspace::End();
	}

	void EditorLayer::OnEvent(Event& event)
	{
		SceneRenderer::OnEvent(event);
	}
}