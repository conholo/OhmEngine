#pragma once

#include "Ohm/Core/Window.h"
#include "Ohm/Event/Event.h"
#include "Ohm/Event/WindowEvent.h"
#include "Ohm/Core/LayerStack.h"
#include "Ohm/ImGui/ImGuiLayer.h"

namespace Ohm
{
	class Application
	{
	public:
		Application(const std::string& name);
		virtual ~Application();

		void Run();
		static void Close();
		void OnEvent(Event& event);

		static Application& GetApplication() { return *s_Instance; }
		Window& GetWindow() const { return *m_Window; }
		const std::string& GetName() const { return m_Name; }

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		ImGuiLayer& GetImGuiLayer() const { return *m_ImGuiLayer; }

	private:
		bool OnWindowClose(WindowClosedEvent& windowCloseEvent);
		bool OnWindowResize(WindowResizedEvent& windowResizeEvent);

	private:
		static Application* s_Instance;

	private:
		std::string m_Name;
		bool m_IsRunning = true;
		Scope<Window> m_Window;
		LayerStack m_LayerStack;
		ImGuiLayer* m_ImGuiLayer;
	};

	Application* CreateApplication();
}