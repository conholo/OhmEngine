#include "EditorLayer.h"


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

	void EditorLayer::OnUpdate()
	{
		if (Input::IsKeyPressed(Key::Space))
		{
			OHM_INFO("On Update");
		}

		RenderCommand::Clear();
		RenderCommand::ClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		Renderer::BeginScene(m_Camera, Primitive::Quad);
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
		
	}
}