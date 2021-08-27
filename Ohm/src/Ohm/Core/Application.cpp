#include "ohmpch.h"
#include "Ohm/Core/Application.h"
#include "Ohm/Rendering/RenderCommand.h"
#include "Ohm/Rendering/Renderer.h"

namespace Ohm
{
	Application* Application::s_Instance = nullptr;

	Application::Application(const std::string& name)
		:m_Name(name)
	{
		s_Instance = this;
		m_Window = CreateScope<Window>(name);
		m_Window->SetEventCallbackFunction(OHM_BIND_FN(Application::OnEvent));
		RenderCommand::Initialize();
		Renderer::Initialize();
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		while (m_IsRunning)
		{
			for (auto* layer : m_LayerStack)
				layer->OnUpdate();

			for (auto* layer : m_LayerStack)
				layer->OnUIRender();


			m_Window->Update();
		}
	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);

		dispatcher.Dispatch<WindowClosedEvent>(OHM_BIND_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizedEvent>(OHM_BIND_FN(Application::OnWindowResize));
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay)
	{
		m_LayerStack.PushOverlay(overlay);
		overlay->OnAttach();
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

