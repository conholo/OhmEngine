#include "ohmpch.h"
#include "Ohm/Core/Application.h"
#include "Ohm/Rendering/RenderCommand.h"
#include "Ohm/Rendering/Renderer.h"
#include "Ohm/Core/Time.h"

#include <GLFW/glfw3.h>

namespace Ohm
{
	Application* Application::s_Instance = nullptr;

	Application::Application(const std::string& name)
		:m_Name(name)
	{
		ASSERT(!s_Instance, "An instance of Application already exists!");
		s_Instance = this;
		m_Window = CreateScope<Window>(name);
		m_Window->SetEventCallbackFunction(OHM_BIND_FN(Application::OnEvent));
		RenderCommand::Initialize();
		RenderCommand::SetViewport(m_Window->GetWidth(), m_Window->GetHeight());
		Renderer::Initialize();

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{
		Renderer::Shutdown();
	}

	void Application::Run()
	{
		while (m_IsRunning)
		{
			Time::Tick();
			for (auto* layer : m_LayerStack)
				layer->OnUpdate(Time::DeltaTime());

			m_ImGuiLayer->Begin();
			for (auto* layer : m_LayerStack)
				layer->OnUIRender();
			m_ImGuiLayer->End();

			m_Window->Update();
		}
	}

	void Application::Close()
	{
		s_Instance->m_IsRunning = false;
	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);

		dispatcher.Dispatch<WindowClosedEvent>(OHM_BIND_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizedEvent>(OHM_BIND_FN(Application::OnWindowResize));

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			if (event.Handled)
				break;
			(*it)->OnEvent(event);
		}
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
	
	bool Application::OnWindowResize(const WindowResizedEvent& windowResizeEvent)
	{
		RenderCommand::SetViewport(windowResizeEvent.GetWidth(), windowResizeEvent.GetHeight());
		return true;
	}
}

