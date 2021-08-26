#include "ohmpch.h"
#include "Ohm/Core/Application.h"

namespace Ohm
{
	Application::Application(const std::string& name)
	{

	}

	Application::~Application()
	{

	}

	void Application::Run()
	{
		while (m_IsRunning);
	}
}

