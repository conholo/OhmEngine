#include "EditorLayer.h"

#include <imgui/imgui.h>

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
		m_Camera.SetPosition(glm::vec3(0.0f, 8.5f, 20.0f));
		m_Camera.SetRotation(glm::vec2(15.0f, 0.0f));
		m_Scene = CreateRef<Scene>("Test Scene");

		// Render Primitives Begin

		// Shaders
		Ref<Shader> blinnPhongShader = CreateRef<Shader>("assets/shaders/Phong.shader");
		Ref<Shader> flatColorShader = CreateRef<Shader>("assets/shaders/flatcolor.shader");
		Ref<Shader> torusShader = CreateRef<Shader>("assets/shaders/Torus.shader");

		// Primitives
		Ref<Mesh> cubeMesh = Mesh::CreatePrimitive(Primitive::Cube);
		Ref<Mesh> sphereMesh = Mesh::CreatePrimitive(Primitive::Sphere);

		// Base Materials
		Ref<Material> baseBlinnPhongMaterial = CreateRef<Material>("Blinn Phong Base", blinnPhongShader);
		Ref<Material> flatColorBaseMaterial = CreateRef<Material>("Flat Color Base", flatColorShader);
		Ref<Material> torusBaseMaterial = CreateRef<Material>("Torus Base", torusShader);

		// Material Instances
		Ref<Material> cubeMaterial = baseBlinnPhongMaterial->Clone("Cube Material");
		Ref<Material> sphereMaterial = baseBlinnPhongMaterial->Clone("Sphere Material");
		Ref<Material> planeMaterial = baseBlinnPhongMaterial->Clone("Plane Material");
		Ref<Material> lightDemoMaterial = flatColorBaseMaterial->Clone("Light Demo Material");
		Ref<Material> torusCubeMaterial = torusBaseMaterial->Clone("Torus Ray March");

		// Render Primitives End

		// Entities
		m_Cube = m_Scene->Create("Cube");
		m_Plane = m_Scene->Create("Plane");
		m_Sphere = m_Scene->Create("Sphere");
		m_Torus = m_Scene->Create("Torus");
		m_DirectionalLight = m_Scene->Create("Directional Light");

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
		m_Plane.AddComponent<MeshRendererComponent>(planeMaterial, cubeMesh);
		m_Torus.AddComponent<MeshRendererComponent>(torusCubeMaterial, cubeMesh);

		lightDemoMaterial->StageUniform<glm::vec4>("u_Color", glm::vec4(1.0f));

		EditorScene::LoadScene(m_Scene);
	}

	void EditorLayer::OnUpdate(Time dt)
	{
		m_ElapsedTime += dt;

		// Sphere
		auto& sphereTransformComponent = m_Sphere.GetComponent<TransformComponent>();
		auto& sphereTranslation = sphereTransformComponent.Translation;
		auto& sphereRotation = sphereTransformComponent.Rotation;
		auto& sphereScale = sphereTransformComponent.Scale;
		sphereTranslation = m_SpherePosition;
		sphereRotation = m_SphereRotation;
		sphereScale = m_SphereSize;

		auto& sphereMaterial = m_Sphere.GetComponent<MeshRendererComponent>().MaterialInstance;
		sphereMaterial->StageUniform<glm::vec4>("u_Color", m_SphereColor);
		sphereMaterial->StageUniform<float>("u_SpecularStrength", m_SphereSpecularStrength);
		sphereMaterial->StageUniform<float>("u_AmbientStrength", m_SphereAmbientStrength);
		sphereMaterial->StageUniform<float>("u_DiffuseStrength", m_SphereDiffuseStrength);
		sphereMaterial->StageUniform<int>("u_Texture", m_SphereIsTextured ? 1 : 0);


		// Cube
		auto& cubeTransformComponent = m_Cube.GetComponent<TransformComponent>();
		auto& cubeTranslation = cubeTransformComponent.Translation;
		auto& cubeRotation = cubeTransformComponent.Rotation;
		auto& cubeScale = cubeTransformComponent.Scale;
		cubeTranslation = m_CubePosition;
		cubeRotation = m_CubeRotation;
		cubeScale = m_CubeSize;

		auto& cubeMaterial = m_Cube.GetComponent<MeshRendererComponent>().MaterialInstance;
		cubeMaterial->StageUniform<glm::vec4>("u_Color", m_CubeColor);
		cubeMaterial->StageUniform<float>("u_SpecularStrength", m_CubeSpecularStrength);
		cubeMaterial->StageUniform<float>("u_AmbientStrength", m_CubeAmbientStrength);
		cubeMaterial->StageUniform<float>("u_DiffuseStrength", m_CubeDiffuseStrength);
		cubeMaterial->StageUniform<int>("u_Texture", m_CubeIsTextured ? 1 : 0);

		// Plane
		auto& planeTransformComponent = m_Plane.GetComponent<TransformComponent>();
		auto& planeTranslation = planeTransformComponent.Translation;
		auto& planeRotation = planeTransformComponent.Rotation;
		auto& planeScale = planeTransformComponent.Scale;
		planeTranslation = m_PlanePosition;
		planeRotation = m_PlaneRotation;
		planeScale = m_PlaneSize;

		auto& planeMaterial = m_Plane.GetComponent<MeshRendererComponent>().MaterialInstance;
		planeMaterial->StageUniform<glm::vec4>("u_Color", m_PlaneColor);
		planeMaterial->StageUniform<float>("u_SpecularStrength", m_PlaneSpecularStrength);
		planeMaterial->StageUniform<float>("u_AmbientStrength", m_PlaneAmbientStrength);
		planeMaterial->StageUniform<float>("u_DiffuseStrength", m_PlaneDiffuseStrength);
		planeMaterial->StageUniform<int>("u_Texture", m_PlaneIsTextured ? 1 : 0);

		// Torus
		auto& torusTransformComponent = m_Torus.GetComponent<TransformComponent>();
		auto& torusTranslation = torusTransformComponent.Translation;
		auto& torusRotation = torusTransformComponent.Rotation;
		auto& torusScale = torusTransformComponent.Scale;
		torusTranslation = m_TorusPosition;
		torusRotation = m_TorusRotation;
		torusScale = m_TorusSize;

		auto& torusMaterial = m_Torus.GetComponent<MeshRendererComponent>().MaterialInstance;
		torusMaterial->StageUniform<glm::vec4>("u_Color", m_TorusColor);

		// Light
		auto& lightTransformComponent = m_DirectionalLight.GetComponent<TransformComponent>();
		auto& lightTranslation = lightTransformComponent.Translation;

		if (m_LightSpin)
		{
			float newX = sin(m_ElapsedTime);
			float newY = cos(m_ElapsedTime);
			float newZ = sin(m_ElapsedTime / 2.0f);
			lightTranslation.x = newX * 15.0f;
			lightTranslation.y = newY * 15.0f;
			lightTranslation.z = newZ * 15.0f;
		}
		else
		{
			auto& lightRotation = lightTransformComponent.Rotation;
			auto& lightScale = lightTransformComponent.Scale;

			lightTranslation = m_LightPosition;
			lightRotation = m_LightRotation;
			lightScale = m_LightSize;
		}

		float& intensity = m_DirectionalLight.GetComponent<LightComponent>().Intensity;

		m_Camera.Update(dt);
		EditorScene::RenderScene(m_Camera);
	}

	void EditorLayer::OnDetach()
	{
		OHM_INFO("On Detach");
	}

	void EditorLayer::OnUIRender()
	{
		static bool dockspaceOpen = true;
		static bool opt_fullscreen = false;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);

		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				// Disabling fullscreen would allow the window to be moved to the front of other windows,
				// which we can't undo at the moment without finer window depth/z control.
				ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
				ImGui::MenuItem("Padding", NULL, &opt_padding);
				ImGui::Separator();

				if (ImGui::MenuItem("Quit"))
				{
					Application::Close();
				}

				ImGui::Separator();

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::Begin("Settings");

		// Light
		ImGui::Separator();
		ImGui::Text("Directional Light");
		ImGui::Separator();
		ImGui::Text("Transform");
		ImGui::DragFloat3("Light Position", &m_LightPosition[0], 0.1f, -100.0f, 100.f);
		ImGui::DragFloat3("Light Rotation", &m_LightRotationDegrees[0], 0.1f);
		m_LightRotation = glm::vec3(glm::radians(m_LightRotationDegrees.x), glm::radians(m_LightRotationDegrees.y), glm::radians(m_LightRotationDegrees.z));
		ImGui::DragFloat3("Light Scale", &m_LightSize[0], 0.1f, -100.0f, 100.f);
		ImGui::Separator();
		ImGui::Text("Lighting");

		auto& light = m_DirectionalLight.GetComponent<LightComponent>();
		ImGui::DragFloat("Light Intensity", &light.Intensity, 0.01f, 0.0f, 1.0f);
		ImGui::ColorPicker4("Light Color", &light.Color[0]);

		ImGui::Checkbox("Light Animation", &m_LightSpin);
		ImGui::Separator();
		ImGui::Separator();


		// Torus
		ImGui::Text("Torus");
		ImGui::Separator();

		ImGui::Text("Transform");
		ImGui::DragFloat3("Torus Position", &m_TorusPosition[0], 0.1f, -100.0f, 100.f);
		ImGui::DragFloat3("Torus Rotation", &m_TorusRotationDegrees[0], 0.1f);
		m_TorusRotation = glm::vec3(glm::radians(m_TorusRotationDegrees.x), glm::radians(m_TorusRotationDegrees.y), glm::radians(m_TorusRotationDegrees.z));
		ImGui::DragFloat3("Torus Scale", &m_TorusSize[0], 0.1f, -100.0f, 100.f);
		ImGui::Separator();

		ImGui::Text("Material");
		ImGui::ColorPicker4("Torus Color", &m_TorusColor[0]);
		ImGui::Separator();
		ImGui::Separator();



		// Sphere
		ImGui::Text("Sphere");
		ImGui::Separator();

		ImGui::Text("Transform");
		ImGui::DragFloat3("Sphere Position", &m_SpherePosition[0], 0.1f, -100.0f, 100.f);
		ImGui::DragFloat3("Sphere Rotation", &m_SphereRotationDegrees[0], 0.1f);
		m_SphereRotation = glm::vec3(glm::radians(m_SphereRotationDegrees.x), glm::radians(m_SphereRotationDegrees.y), glm::radians(m_SphereRotationDegrees.z));
		ImGui::DragFloat3("Sphere Scale", &m_SphereSize[0], 0.1f, -100.0f, 100.f);
		ImGui::Separator();

		ImGui::Text("Material");
		ImGui::ColorPicker4("Sphere Color", &m_SphereColor[0]);
		ImGui::DragFloat("Sphere Specular Strength", &m_SphereSpecularStrength, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Sphere Ambient Strength", &m_SphereAmbientStrength, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Sphere Diffuse Strength", &m_SphereDiffuseStrength, 0.01f, 0.0f, 1.0f);
		ImGui::Checkbox("Sphere Textured", &m_SphereIsTextured);
		ImGui::Separator();
		ImGui::Separator();

		// Cube
		ImGui::Text("Cube");
		ImGui::Separator();

		ImGui::Text("Transform");
		ImGui::DragFloat3("Cube Position", &m_CubePosition[0], 0.1f, -100.0f, 100.f);
		ImGui::DragFloat3("Cube Rotation", &m_CubeRotationDegrees[0], 0.1f);
		m_CubeRotation = glm::vec3(glm::radians(m_CubeRotationDegrees.x), glm::radians(m_CubeRotationDegrees.y), glm::radians(m_CubeRotationDegrees.z));
		ImGui::DragFloat3("Cube Scale", &m_CubeSize[0], 0.1f, -100.0f, 100.f);
		ImGui::Separator();

		ImGui::Text("Material");
		ImGui::ColorPicker4("Cube Color", &m_CubeColor[0]);
		ImGui::DragFloat("Cube Specular Strength", &m_CubeSpecularStrength, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Cube Ambient Strength", &m_CubeAmbientStrength, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Cube Diffuse Strength", &m_CubeDiffuseStrength, 0.01f, 0.0f, 1.0f);
		ImGui::Checkbox("Cube Textured", &m_CubeIsTextured);
		ImGui::Separator();
		ImGui::Separator();

		// Plane
		ImGui::Text("Plane");
		ImGui::Separator();

		ImGui::Text("Transform");
		ImGui::DragFloat3("Plane Position", &m_PlanePosition[0], 0.1f, -100.0f, 100.f);
		ImGui::DragFloat3("Plane Rotation", &m_PlaneRotationDegrees[0], 0.1f);
		m_PlaneRotation = glm::vec3(glm::radians(m_PlaneRotationDegrees.x), glm::radians(m_PlaneRotationDegrees.y), glm::radians(m_PlaneRotationDegrees.z));
		ImGui::DragFloat3("Plane Scale", &m_PlaneSize[0], 0.1f, -100.0f, 100.f);
		ImGui::Separator();

		ImGui::Text("Material");
		ImGui::ColorPicker4("Plane Color", &m_PlaneColor[0]);
		ImGui::DragFloat("Plane Specular Strength", &m_PlaneSpecularStrength, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Plane Ambient Strength", &m_PlaneAmbientStrength, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Plane Diffuse Strength", &m_PlaneDiffuseStrength, 0.01f, 0.0f, 1.0f);
		ImGui::Checkbox("Plane Textured", &m_PlaneIsTextured);
		ImGui::Separator();
		ImGui::Separator();

		ImGui::End();

		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& event)
	{
		m_Camera.OnEvent(event);

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowResizedEvent>(OHM_BIND_FN(EditorLayer::OnWindowResized));
	}

	bool EditorLayer::OnWindowResized(WindowResizedEvent& windowResizedEvent)
	{
		m_Camera.SetViewportSize((float)windowResizedEvent.GetWidth(), (float)windowResizedEvent.GetHeight());
		return true;
	}
}