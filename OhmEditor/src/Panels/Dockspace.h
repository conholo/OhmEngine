#pragma once


namespace Ohm
{
	class Dockspace
	{
		typedef int ImGuiDockNodeFlags;

	public:
		static void Begin();
		static void Draw();
		static void End();
	private:
		static ImGuiDockNodeFlags s_DockspaceFlags;
		static float s_GlobalMinWindowWidth;
	};
}