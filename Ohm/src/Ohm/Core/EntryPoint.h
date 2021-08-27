#pragma once

extern Ohm::Application* Ohm::CreateApplication();

int main()
{
	Ohm::Log::Init();
	auto* app = Ohm::CreateApplication();
	app->Run();
	delete app;
}