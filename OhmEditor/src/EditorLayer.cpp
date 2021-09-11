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
		m_TestTexture = CreateRef<Texture2D>("assets/textures/lava.jpg");

		// Render Primitives Begin

		// Shaders
		Ref<Shader> blinnPhongShader = CreateRef<Shader>("assets/shaders/Phong.shader");
		Ref<Shader> flatColorShader = CreateRef<Shader>("assets/shaders/flatcolor.shader");
		Ref<Shader> torusShader = CreateRef<Shader>("assets/shaders/Torus.shader");

		// Primitives
		Ref<Mesh> cubeMesh = Mesh::CreatePrimitive(Primitive::Cube);
		Ref<Mesh> quadMesh = Mesh::CreatePrimitive(Primitive::Quad);
		Ref<Mesh> planeMesh = Mesh::CreatePrimitive(Primitive::Plane);
		Ref<Mesh> sphereMesh = Mesh::CreatePrimitive(Primitive::Sphere);

		// Base Materials
		Ref<Material> baseBlinnPhongMaterial = CreateRef<Material>("Blinn Phong Base", blinnPhongShader);
		Ref<Material> flatColorBaseMaterial = CreateRef<Material>("Flat Color Base", flatColorShader);
		Ref<Material> torusBaseMaterial = CreateRef<Material>("Torus Base", torusShader);

		// Material Instances
		Ref<Material> cubeMaterial = baseBlinnPhongMaterial->Clone("Cube Material");
		Ref<Material> sphereMaterial = baseBlinnPhongMaterial->Clone("Sphere Material");
		Ref<Material> planeMaterial = baseBlinnPhongMaterial->Clone("Plane Material");
		Ref<Material> quadMaterial = baseBlinnPhongMaterial->Clone("Quad Material");
		Ref<Material> lightDemoMaterial = flatColorBaseMaterial->Clone("Light Demo Material");
		Ref<Material> torusCubeMaterial = torusBaseMaterial->Clone("Torus Ray March");

		// Render Primitives End

		// Entities
		m_Cube = m_Scene->Create("Cube");
		m_Plane = m_Scene->Create("Plane");
		m_Sphere = m_Scene->Create("Sphere");
		m_Torus = m_Scene->Create("Torus");
		m_Quad = m_Scene->Create("Quad");
		m_DirectionalLight = m_Scene->Create("Directional Light");

		m_CubeWrapper = CreateRef<TransformWrapper>(m_Cube.GetComponent<TransformComponent>(), glm::vec3(5.0f, 3.0f, 0.0), glm::radians(glm::vec3(45.0f)), glm::vec3(3.5f));
		m_PlaneWrapper = CreateRef<TransformWrapper>(m_Plane.GetComponent<TransformComponent>(), glm::vec3(0.0f, -2.0f, 0.0), glm::vec3(0.0f), glm::vec3(100.0f, 0.01f, 100.0f));
		m_SphereWrapper = CreateRef<TransformWrapper>(m_Sphere.GetComponent<TransformComponent>(), glm::vec3(-5.0f, 3.0f, 0.0), glm::vec3(0.0f), glm::vec3(2.0f));
		m_QuadWrapper = CreateRef<TransformWrapper>(m_Quad.GetComponent<TransformComponent>(), glm::vec3(0.0f, 8.0f, 0.0), glm::radians(glm::vec3(-45.0f, 0.0f, 0.0f)), glm::vec3(3.0f));
		m_TorusWrapper = CreateRef<TransformWrapper>(m_Torus.GetComponent<TransformComponent>(), glm::vec3(0.0f, 3.0f, 0.0), glm::vec3(0.0f) , glm::vec3(5.0f));

		// Light
		m_DirectionalLight.AddComponent<LightComponent>(LightType::Directional, m_LightColor, 1.0f, true);
		m_DirectionalLight.AddComponent<MeshRendererComponent>(lightDemoMaterial, cubeMesh);
		auto& lightTranslation = m_DirectionalLight.GetComponent<TransformComponent>().Translation;
		lightTranslation = m_LightPosition;
		auto& lightScale = m_DirectionalLight.GetComponent<TransformComponent>().Scale;
		lightScale = m_LightSize;
		auto& lightRotation = m_DirectionalLight.GetComponent<TransformComponent>().Rotation;
		lightRotation = glm::vec3(glm::radians(m_LightRotationDegrees.x), glm::radians(m_LightRotationDegrees.y), glm::radians(m_LightRotationDegrees.z));

		// Scene Entities
		m_Sphere.AddComponent<MeshRendererComponent>(sphereMaterial, sphereMesh);
		m_Cube.AddComponent<MeshRendererComponent>(cubeMaterial, cubeMesh);
		m_Plane.AddComponent<MeshRendererComponent>(planeMaterial, planeMesh);
		m_Torus.AddComponent<MeshRendererComponent>(torusCubeMaterial, cubeMesh);
		m_Quad.AddComponent<MeshRendererComponent>(quadMaterial, quadMesh);

		lightDemoMaterial->StageUniform<glm::vec4>("u_Color", glm::vec4(1.0f));

		quadMaterial->StageUniform<glm::vec4>("u_Color", m_QuadColor);
		quadMaterial->StageUniform<float>("u_SpecularStrength", m_QuadSpecularStrength);
		quadMaterial->StageUniform<float>("u_AmbientStrength", m_QuadAmbientStrength);
		quadMaterial->StageUniform<float>("u_DiffuseStrength", m_QuadDiffuseStrength);
		quadMaterial->StageUniform<int>("u_Texture", m_QuadIsTextured ? 1 : 0);


		// Sphere
		sphereMaterial->StageUniform<glm::vec4>("u_Color", m_SphereColor);
		sphereMaterial->StageUniform<float>("u_SpecularStrength", m_SphereSpecularStrength);
		sphereMaterial->StageUniform<float>("u_AmbientStrength", m_SphereAmbientStrength);
		sphereMaterial->StageUniform<float>("u_DiffuseStrength", m_SphereDiffuseStrength);
		sphereMaterial->StageUniform<int>("u_Texture", m_SphereIsTextured ? 1 : 0);


		// Cube
		cubeMaterial->StageUniform<glm::vec4>("u_Color", m_CubeColor);
		cubeMaterial->StageUniform<float>("u_SpecularStrength", m_CubeSpecularStrength);
		cubeMaterial->StageUniform<float>("u_AmbientStrength", m_CubeAmbientStrength);
		cubeMaterial->StageUniform<float>("u_DiffuseStrength", m_CubeDiffuseStrength);
		cubeMaterial->StageUniform<int>("u_Texture", m_CubeIsTextured ? 1 : 0);

		// Plane
		planeMaterial->StageUniform<glm::vec4>("u_Color", m_PlaneColor);
		planeMaterial->StageUniform<float>("u_SpecularStrength", m_PlaneSpecularStrength);
		planeMaterial->StageUniform<float>("u_AmbientStrength", m_PlaneAmbientStrength);
		planeMaterial->StageUniform<float>("u_DiffuseStrength", m_PlaneDiffuseStrength);
		planeMaterial->StageUniform<int>("u_Texture", m_PlaneIsTextured ? 1 : 0);

		// Torus
		torusCubeMaterial->StageUniform<glm::vec4>("u_Color", m_TorusColor);
		torusCubeMaterial->StageUniform<float>("u_SmoothBlend", m_TorusSmoothBlend);


		float& intensity = m_DirectionalLight.GetComponent<LightComponent>().Intensity;


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