#pragma once

#include "Ohm/Core/Window.h"
#include "Ohm/Event/Event.h"
#include "Ohm/Event/WindowEvent.h"

namespace Ohm
{
	class Application
	{
	public:
		Application(const std::string& name);
		virtual ~Application();

		void Run();
		void OnEvent(Event& event);

		static Application& GetApplication() { return *s_Instance; }
		Window& GetWindow() const { return *m_Window; }

	private:
		bool OnWindowClose(WindowClosedEvent& windowCloseEvent);
		bool OnWindowResize(WindowResizedEvent& windowResizeEvent);

	private:
		static Application* s_Instance;

	private:
		bool m_IsRunning = true;
		Scope<Window> m_Window;
	};

	Application* CreateApplication();
}