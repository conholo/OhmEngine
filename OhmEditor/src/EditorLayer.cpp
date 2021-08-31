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
		m_DirectionalLight = m_Scene->Create("Directional Light");

		m_DirectionalLight.AddComponent<LightComponent>(LightType::Directional);
		auto& lightTranslation = m_DirectionalLight.GetComponent<TransformComponent>().Translation;
		lightTranslation = glm::vec3(0.0f, 10.0f, 0.0f);

		Ref<Shader> blinnPhongShader = CreateRef<Shader>("assets/shaders/Blinn-Phong.shader");
		Ref<Mesh> cubeMesh = Mesh::CreatePrimitive(Primitive::Cube);

		blueSquare.AddComponent<MeshRendererComponent>(blinnPhongShader, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), cubeMesh);
		blueRectangle.AddComponent<MeshRendererComponent>(blinnPhongShader, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), cubeMesh);

		auto& squareTranslation = blueSquare.GetComponent<TransformComponent>().Translation;
		squareTranslation = m_QuadPosition;
		auto& squareRotation = blueSquare.GetComponent<TransformComponent>().Rotation;
		squareRotation = glm::vec3(0.0f, 0.0f, 0.0f);

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
		lightTranslation.x = newX * 10.0f;
		lightTranslation.y = newY * 10.0f;

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
	}
}