#include "EditorLayer.h"

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
		OHM_INFO("On Attach");
	}

	void EditorLayer::OnUpdate()
	{
		if (Input::IsKeyPressed(Key::Space))
		{
			OHM_INFO("On Update");
		}
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