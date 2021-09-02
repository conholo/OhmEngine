#include "EditorLayer.h"

#include <imgui/imgui.h>

#include "Ohm/Editor/EditorScene.h"
#include <glm/glm.hpp>

#include <cmath>

namespace Ohm
{
	EditorLayer::EditorLayer()
		:Layer("Editor Layer"), m_Camera(45.0f, 1.0f, 0.1f, 1000.0f)
	{

	}

	EditorLayer::~EditorLayer()
	{

	}

	void EditorLayer::OnAttach()
	{
		m_Scene = CreateRef<Scene>("Test Scene");

		m_Cube = m_Scene->Create("Cube");
		Entity blueRectangle = m_Scene->Create("Blue Rectangle");
		m_Sphere = m_Scene->Create("Sphere");
		m_DirectionalLight = m_Scene->Create("Directional Light");

		Ref<Shader> blinnPhongShader = CreateRef<Shader>("assets/shaders/Blinn-Phong.shader");
		Ref<Shader> flatColorShader = CreateRef<Shader>("assets/shaders/flatcolor.shader");
		Ref<Mesh> cubeMesh = Mesh::CreatePrimitive(Primitive::Cube);
		Ref<Mesh> sphereMesh = Mesh::CreatePrimitive(Primitive::Sphere);

		// Light
		m_DirectionalLight.AddComponent<LightComponent>(LightType::Directional, true);
		m_DirectionalLight.AddComponent<MeshRendererComponent>(flatColorShader, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), cubeMesh);
		auto& lightTranslation = m_DirectionalLight.GetComponent<TransformComponent>().Translation;
		lightTranslation = m_LightPosition;
		auto& lightScale = m_DirectionalLight.GetComponent<TransformComponent>().Scale;
		lightScale = m_LightSize;
		auto& lightRotation = m_DirectionalLight.GetComponent<TransformComponent>().Rotation;
		lightRotation = glm::vec3(glm::radians(m_LightRotationDegrees.x), glm::radians(m_LightRotationDegrees.y), glm::radians(m_LightRotationDegrees.z));


		m_Sphere.AddComponent<MeshRendererComponent>(blinnPhongShader, glm::vec4(0.0f, 0.3f, 0.8f, 1.0f), sphereMesh);
		m_Cube.AddComponent<MeshRendererComponent>(blinnPhongShader, glm::vec4(0.8f, 0.3f, 0.1f, 1.0f), cubeMesh);
		blueRectangle.AddComponent<MeshRendererComponent>(blinnPhongShader, glm::vec4(0.1f, .8f, 0.0f, 1.0f), cubeMesh);

		auto& sphereTranslation = m_Sphere.GetComponent<TransformComponent>().Translation;
		sphereTranslation = m_SpherePosition;

		auto& cubeTranslation = m_Cube.GetComponent<TransformComponent>().Translation;
		cubeTranslation = m_CubePosition;
		auto& cubeRotation = m_Cube.GetComponent<TransformComponent>().Rotation;
		m_CubeRotation = glm::vec3(glm::radians(m_CubeRotationDegrees.x), glm::radians(m_CubeRotationDegrees.y), glm::radians(m_CubeRotationDegrees.z));
		cubeRotation = m_CubeRotation;
		auto& cubeScale = m_Cube.GetComponent<TransformComponent>().Scale;
		cubeScale = m_CubeSize;

		auto& rectangleTranslation = blueRectangle.GetComponent<TransformComponent>().Translation;
		rectangleTranslation = m_PlanePosition;
		auto& rectangleSize = blueRectangle.GetComponent<TransformComponent>().Scale;
		rectangleSize = m_PlaneSize;

		EditorScene::LoadScene(m_Scene);
	}

	void EditorLayer::OnUpdate(Time dt)
	{
		m_ElapsedTime += dt;

		auto& sphereTransformComponent = m_Sphere.GetComponent<TransformComponent>();
		auto& sphereTranslation = sphereTransformComponent.Translation;
		auto& sphereScale = sphereTransformComponent.Scale;
		sphereTranslation = m_SpherePosition;
		sphereScale = m_SphereSize;
		auto& sphereMeshComponent = m_Sphere.GetComponent<MeshRendererComponent>();
		sphereMeshComponent.Color = m_SphereColor;

		auto& cubeTransformComponent = m_Cube.GetComponent<TransformComponent>();
		auto& cubeTranslation = cubeTransformComponent.Translation;
		auto& cubeRotation = cubeTransformComponent.Rotation;
		auto& cubeScale = cubeTransformComponent.Scale;
		cubeTranslation = m_CubePosition;
		cubeRotation = m_CubeRotation;
		cubeScale = m_CubeSize;
		auto& cubeMeshComponent = m_Cube.GetComponent<MeshRendererComponent>();
		cubeMeshComponent.Color = m_CubeColor;


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


		m_Camera.Update(dt);
		EditorScene::RenderScene(m_Camera, m_DirectionalLight);
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

		ImGui::Text("Here are some neat settings!");

		ImGui::Text("Directional Light");
		ImGui::DragFloat3("Light Position", &m_LightPosition[0], 0.1f, -100.0f, 100.f);
		ImGui::DragFloat3("Light Rotation", &m_LightRotationDegrees[0], 0.1f);
		m_LightRotation = glm::vec3(glm::radians(m_LightRotationDegrees.x), glm::radians(m_LightRotationDegrees.y), glm::radians(m_LightRotationDegrees.z));
		ImGui::DragFloat3("Light Scale", &m_LightSize[0], 0.1f, -100.0f, 100.f);
		ImGui::Checkbox("Light Animation", &m_LightSpin);

		ImGui::Text("Sphere");
		ImGui::DragFloat3("Sphere Position", &m_SpherePosition[0], 0.1f, -100.0f, 100.f);
		ImGui::DragFloat3("Sphere Scale", &m_SphereSize[0], 0.1f, -100.0f, 100.f);
		ImGui::ColorPicker4("Sphere Color", &m_SphereColor[0]);

		ImGui::Text("Cube");
		ImGui::DragFloat3("Cube Position", &m_CubePosition[0], 0.1f, -100.0f, 100.f);

		ImGui::DragFloat3("Cube Rotation", &m_CubeRotationDegrees[0], 0.1f);
		m_CubeRotation = glm::vec3(glm::radians(m_CubeRotationDegrees.x), glm::radians(m_CubeRotationDegrees.y), glm::radians(m_CubeRotationDegrees.z));
		ImGui::DragFloat3("Cube Scale", &m_CubeSize[0], 0.1f, -100.0f, 100.f);
		ImGui::ColorPicker4("Cube Color", &m_CubeColor[0]);


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