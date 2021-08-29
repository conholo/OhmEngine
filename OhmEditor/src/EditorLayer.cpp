#include "EditorLayer.h"

#include <glm/glm.hpp>


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

	}

	void EditorLayer::OnUpdate(Time dt)
	{
		if (Input::IsKeyPressed(Key::Space))
		{
			OHM_INFO("On Update");
		}

		m_Camera.Update(dt);

		RenderCommand::Clear();
		RenderCommand::ClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		Renderer::BeginScene(m_Camera, Primitive::Quad);
		Renderer::UploadModelData(m_QuadPosition, m_QuadSize);
		Renderer::EndScene();

		Renderer::BeginScene(m_Camera, Primitive::Quad);
		Renderer::UploadModelData(m_PlanePosition, m_PlaneSize);
		Renderer::EndScene();

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