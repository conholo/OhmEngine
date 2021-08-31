#include "EditorLayer.h"

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

		Entity blueSquare = m_Scene->Create("Blue Square");
		Entity blueRectangle = m_Scene->Create("Blue Rectangle");
		Entity sphere = m_Scene->Create("Sphere");
		m_DirectionalLight = m_Scene->Create("Directional Light");

		Ref<Shader> blinnPhongShader = CreateRef<Shader>("assets/shaders/Blinn-Phong.shader");
		Ref<Shader> flatColorShader = CreateRef<Shader>("assets/shaders/flatcolor.shader");
		Ref<Mesh> cubeMesh = Mesh::CreatePrimitive(Primitive::Cube);
		Ref<Mesh> sphereMesh = Mesh::CreatePrimitive(Primitive::Sphere);

		// Light
		m_DirectionalLight.AddComponent<LightComponent>(LightType::Directional, true);
		m_DirectionalLight.AddComponent<MeshRendererComponent>(flatColorShader, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), cubeMesh);
		auto& lightTranslation = m_DirectionalLight.GetComponent<TransformComponent>().Translation;
		lightTranslation = glm::vec3(0.0f, 5.0f, 0.0f);
		auto& lightScale = m_DirectionalLight.GetComponent<TransformComponent>().Scale;
		lightScale = glm::vec3(0.5f, 0.5f, 0.5f);

		sphere.AddComponent<MeshRendererComponent>(blinnPhongShader, glm::vec4(0.0f, 0.3f, 0.8f, 1.0f), sphereMesh);
		blueSquare.AddComponent<MeshRendererComponent>(blinnPhongShader, glm::vec4(0.8f, 0.3f, 0.1f, 1.0f), cubeMesh);
		blueRectangle.AddComponent<MeshRendererComponent>(blinnPhongShader, glm::vec4(0.1f, .8f, 0.0f, 1.0f), cubeMesh);

		auto& sphereTranslation = sphere.GetComponent<TransformComponent>().Translation;
		sphereTranslation = glm::vec3(0.0f, 2.5f, 0.0f);

		auto& squareTranslation = blueSquare.GetComponent<TransformComponent>().Translation;
		squareTranslation = m_QuadPosition;
		auto& squareRotation = blueSquare.GetComponent<TransformComponent>().Rotation;
		squareRotation = glm::vec3(glm::radians(45.0f), glm::radians(45.0f), 0.0f);
		auto& squareScale = blueSquare.GetComponent<TransformComponent>().Scale;
		squareScale = glm::vec3(2.0f, 2.0f, 2.0f);

		auto& rectangleTranslation = blueRectangle.GetComponent<TransformComponent>().Translation;
		rectangleTranslation = m_PlanePosition;
		auto& rectangleSize = blueRectangle.GetComponent<TransformComponent>().Scale;
		rectangleSize = m_PlaneSize;

		EditorScene::LoadScene(m_Scene);
	}

	void EditorLayer::OnUpdate(Time dt)
	{
		m_ElapsedTime += dt;
		auto& lightTranslation = m_DirectionalLight.GetComponent<TransformComponent>().Translation;

		float newX = sin(m_ElapsedTime);
		float newY = cos(m_ElapsedTime);
		float newZ = sin(m_ElapsedTime / 2.0f);
		lightTranslation.x = newX * 15.0f;
		lightTranslation.y = newY * 15.0f;
		lightTranslation.z = newZ * 15.0f;

		m_Camera.Update(dt);
		EditorScene::RenderScene(m_Camera, m_DirectionalLight);
	}

	void EditorLayer::OnDetach()
	{
		OHM_INFO("On Detach");
	}

	void EditorLayer::OnUIRender()
	{

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