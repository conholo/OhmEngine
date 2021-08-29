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

			RenderCommand::Clear();
			RenderCommand::ClearColor(0.1f, 0.1f, 0.1f, 1.0f);

			for (auto* layer : m_LayerStack)
				layer->OnUpdate(deltaTime);

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

		for (Layer* layer : m_LayerStack)
			layer->OnEvent(event);
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

