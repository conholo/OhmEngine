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
		s_Instance = this;
		m_Window = CreateScope<Window>(name);
		m_Window->SetEventCallbackFunction(OHM_BIND_FN(Application::OnEvent));
		RenderCommand::Initialize();
		Renderer::Initialize();

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		while (m_IsRunning)
		{
			float time = (float)glfwGetTime();

			Time deltaTime = time - m_LastFrameTime;
			m_LastFrameTime = time;

			for (auto* layer : m_LayerStack)
				layer->OnUpdate(deltaTime);

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
	
	bool Application::OnWindowResize(WindowResizedEvent& windowResizeEvent)
	{
		RenderCommand::SetViewport(windowResizeEvent.GetWidth(), windowResizeEvent.GetHeight());

		return true;
	}
}

