#pragma once

#include "Ohm/Core/Application.h"

extern Ohm::Application* Ohm::CreateApplication();

int main()
{
	auto* app = Ohm::CreateApplication();
	app->Run();
	delete app;
}