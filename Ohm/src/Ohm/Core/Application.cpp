#include "ohmpch.h"
#include "Ohm/Core/Application.h"
#include "Ohm/Rendering/RenderCommand.h"

namespace Ohm
{
	Application* Application::s_Instance = nullptr;

	Application::Application(const std::string& name)
	{
		s_Instance = this;
		m_Window = CreateScope<Window>(name);
		m_Window->SetEventCallbackFunction(OHM_BIND_FN(Application::OnEvent));
		RenderCommand::Initialize();
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		while (m_IsRunning)
		{
			RenderCommand::Clear();
			RenderCommand::ClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			m_Window->Update();
		}
	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);

		std::cout << event << std::endl;

		dispatcher.Dispatch<WindowClosedEvent>(OHM_BIND_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizedEvent>(OHM_BIND_FN(Application::OnWindowResize));
	}

	bool Application::OnWindowClose(WindowClosedEvent& windowCloseEvent)
	{
		m_IsRunning = false;
		return true;
	}
	
	bool Application::OnWindowResize(WindowResizedEvent& windowResizeEvent)
	{
		RenderCommand::SetViewport(windowResizeEvent.GetWidth(), windowResizeEvent.GetHeight());

		return true;
	}


}

