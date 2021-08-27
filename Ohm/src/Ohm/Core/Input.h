#pragma once

#include "Ohm/Core/KeyCodes.h"
#include "Ohm/Core/MouseCodes.h"


namespace Ohm
{
	class Input
	{
	public:

		static bool IsKeyPressed(KeyCode key);
		static bool IsMouseButtonPressed(MouseCode button);
		
		static float GetMouseX();
		static float GetMouseY();
	};
}