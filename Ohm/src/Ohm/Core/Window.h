#pragma once

#include "Ohm/Event/Event.h"

struct GLFWwindow;

namespace Ohm
{

	class Window
	{
	public:

		using EventCallbackFunction = std::function<void(Event&)>;

		Window(const std::string& name = "Ohm Engine", uint32_t width = 800, uint32_t height = 800);
		~Window();

		void Update();
		void SetEventCallbackFunction(const EventCallbackFunction& callback) { m_WindowData.Callback = callback; }


		uint32_t GetWidth() const { return m_WindowData.Width; }
		uint32_t GetHeight() const { return m_WindowData.Height; }
		uint32_t GetScaledWidth() const { return m_WindowData.ScaledWidth; }
		uint32_t GetScaledHeight() const { return m_WindowData.ScaledHeight; }

		void SetVSync(bool enable);
		bool IsVSync() const { return m_WindowData.VSync; }

		GLFWwindow* GetWindowHandle() const { return m_WindowHandle; }

	private:
		void Initialize();
		void Shutdown();

	private:
		GLFWwindow* m_WindowHandle;

		struct WindowData
		{
			std::string Name;
			uint32_t Width, Height;
			uint32_t ScaledWidth, ScaledHeight;
			EventCallbackFunction Callback;
			bool VSync;

			WindowData(const std::string& name = "Ohm Engine", uint32_t width = 800, uint32_t height = 800, bool vSync = true)
				:Name(name), Width(width), Height(height), VSync(vSync){ }
		};

		WindowData m_WindowData;
	};
}