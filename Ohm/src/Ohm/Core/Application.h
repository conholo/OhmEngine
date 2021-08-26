#pragma once

namespace Ohm
{
	class Application
	{
	public:
		Application(const std::string& name);
		virtual ~Application();

		void Run();

	private:
		bool m_IsRunning = true;
	};

	Application* CreateApplication();
}