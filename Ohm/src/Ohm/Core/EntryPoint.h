#pragma once

extern Ohm::Application* Ohm::CreateApplication();

int main()
{
	auto* app = Ohm::CreateApplication();
	app->Run();
	delete app;
}