#include "EditorLayer.h"

#include <imgui/imgui.h>

#include "Panels/Dockspace.h"

#include "Ohm/Editor/EditorScene.h"
#include <glm/glm.hpp>

#include <cmath>

namespace Ohm
{
	EditorLayer::EditorLayer()
		:Layer("Editor Layer"), m_Camera(45.0f, 1.77778f, 0.1f, 1000.0f)
	{

	}

	EditorLayer::~EditorLayer()
	{

	}

	void EditorLayer::OnAttach()
	{
		FramebufferSpecification framebufferSpecification;
		framebufferSpecification.AttachmentData = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
		framebufferSpecification.Width = 1280;
		framebufferSpecification.Height = 720;
		
		m_Framebuffer = CreateRef<Framebuffer>(framebufferSpecification);
		m_ViewportPanel.SetFramebuffer(m_Framebuffer);

		m_Camera.SetPosition(glm::vec3(0.0f, 8.5f, 20.0f));
		m_Camera.SetRotation(glm::vec2(15.0f, 0.0f));
		m_Scene = CreateRef<Scene>("Test Scene");

		//-----------------------------------------------------
		// Render Primitives Begin
		//-----------------------------------------------------
		// Shaders
		Ref<Shader> blinnPhongShader = CreateRef<Shader>("assets/shaders/Phong.shader");
		Ref<Shader> flatColorShader = CreateRef<Shader>("assets/shaders/flatcolor.shader");

		// Primitives
		Ref<Mesh> cubeMesh = Mesh::CreatePrimitive(Primitive::Cube);
		Ref<Mesh> quadMesh = Mesh::CreatePrimitive(Primitive::Quad);
		Ref<Mesh> planeMesh = Mesh::CreatePrimitive(Primitive::Plane);
		Ref<Mesh> sphereMesh = Mesh::CreatePrimitive(Primitive::Sphere);

		// Materials
		Ref<Material> cubeMaterial = CreateRef<Material>("Cube Material", blinnPhongShader);
		Ref<Material> sphereMaterial = CreateRef<Material>("Sphere Material", blinnPhongShader);
		Ref<Material> planeMaterial = CreateRef<Material>("Plane Material", blinnPhongShader);
		Ref<Material> quadMaterial = CreateRef<Material>("Quad Material", blinnPhongShader);
		Ref<Material> lightDemoMaterial = CreateRef<Material>("Light Demo Material", flatColorShader);
		Ref<Material> testFlatColorMaterial = CreateRef<Material>("Flat Color", flatColorShader);
		//-----------------------------------------------------
		// Render Primitives End
		//-----------------------------------------------------


		//-----------------------------------------------------
		// Entities Begin
		//-----------------------------------------------------
		m_Cube = m_Scene->Create("Cube");
		m_Plane = m_Scene->Create("Plane");
		m_Sphere = m_Scene->Create("Sphere");
		m_Quad = m_Scene->Create("Quad");
		m_TestFlatColorCube = m_Scene->Create("FC Cube");
		m_DirectionalLight = m_Scene->Create("Directional Light");

		// Light
		m_DirectionalLight.AddComponent<LightComponent>(LightType::Directional, m_LightColor, 1.0f, true);
		// TODO:: NOTE TO FUTURE SELF, IF LIGHTING DATA FAILS, IT'S BECAUSE A MESH RENDERER IS CURRENTLY NEEDEED TO ACCESS SCENE DIRECTIONAL LIGHT -> CHECK EDITORSCENE
		m_DirectionalLight.AddComponent<MeshRendererComponent>(lightDemoMaterial, cubeMesh);

		m_Sphere.AddComponent<MeshRendererComponent>(sphereMaterial, sphereMesh);
		m_Cube.AddComponent<MeshRendererComponent>(cubeMaterial, cubeMesh);
		m_Plane.AddComponent<MeshRendererComponent>(planeMaterial, planeMesh);
		m_Quad.AddComponent<MeshRendererComponent>(quadMaterial, quadMesh);
		m_TestFlatColorCube.AddComponent<MeshRendererComponent>(testFlatColorMaterial, cubeMesh);

		//-----------------------------------------------------
		// Entities End
		//-----------------------------------------------------


		//-----------------------------------------------------
		// Test Transform Wrappers (DELETE THESE)
		//-----------------------------------------------------
		m_CubeWrapper = CreateRef<TransformWrapper>(m_Cube.GetComponent<TransformComponent>(), glm::vec3(7.0f, 3.0f, 0.0), glm::radians(glm::vec3(45.0f)), glm::vec3(3.5f));
		m_PlaneWrapper = CreateRef<TransformWrapper>(m_Plane.GetComponent<TransformComponent>(), glm::vec3(0.0f, -2.0f, 0.0), glm::vec3(0.0f), glm::vec3(100.0f, 0.01f, 100.0f));
		m_SphereWrapper = CreateRef<TransformWrapper>(m_Sphere.GetComponent<TransformComponent>(), glm::vec3(-7.0f, 3.0f, 0.0), glm::vec3(0.0f), glm::vec3(2.0f));
		m_QuadWrapper = CreateRef<TransformWrapper>(m_Quad.GetComponent<TransformComponent>(), glm::vec3(0.0f, 3.0f, 0.0), glm::radians(glm::vec3(-45.0f, 0.0f, 0.0f)), glm::vec3(3.0f));
		m_LightDemoWrapper = CreateRef<TransformWrapper>(m_DirectionalLight.GetComponent<TransformComponent>(), glm::vec3(0.0f, 75.0f, 0.0), glm::vec3(0.0f), glm::vec3(.5f));
		//-----------------------------------------------------
		// Test Transform Wrappers (DELETE THESE)
		//-----------------------------------------------------


		m_SceneHierarchyPanel.SetContext(m_Scene);
		EditorScene::LoadScene(m_Scene);
	}

	void EditorLayer::OnUpdate(Time dt)
	{
		glm::vec2 viewportSize = m_ViewportPanel.GetViewportSize();

		if (FramebufferSpecification specification = m_Framebuffer->GetFrameBufferSpecification();
			viewportSize.x > 0.0f && viewportSize.y > 0.0f &&
			(specification.Width != viewportSize.x || specification.Height != viewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
			m_Camera.SetViewportSize(viewportSize.x, viewportSize.y);
		}

		m_Camera.Update(dt);

		m_Framebuffer->Bind();
		RenderCommand::Clear();
		RenderCommand::ClearColor(m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a);
		EditorScene::RenderScene(m_Camera);

		m_Framebuffer->Unbind();
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
		m_ViewportPanel.Draw();
		Dockspace::End();
	}

	void EditorLayer::OnEvent(Event& event)
	{
		m_Camera.OnEvent(event);
	}
}