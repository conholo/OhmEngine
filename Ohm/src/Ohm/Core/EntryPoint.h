#pragma once

extern Ohm::Application* Ohm::CreateApplication();

int main()
{
	auto* app = Ohm::CreateApplication();
	Ohm::Log::Init(app->GetName());
	app->Run();
	delete app;
}