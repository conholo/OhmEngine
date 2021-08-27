#include "ohmpch.h"
#include "Ohm/Core/Input.h"
#include "Ohm/Core/Application.h"

#include <GLFW/glfw3.h>

namespace Ohm
{
	bool Input::IsKeyPressed(KeyCode key)
	{
		GLFWwindow* window = Application::GetApplication().GetWindow().GetWindowHandle();
		int action = glfwGetKey(window, key);

		return action == GLFW_PRESS || action == GLFW_REPEAT;
	}

	bool Input::IsMouseButtonPressed(MouseCode button)
	{
		GLFWwindow* window = Application::GetApplication().GetWindow().GetWindowHandle();
		int action = glfwGetMouseButton(window, button);

		return action == GLFW_PRESS || action == GLFW_REPEAT;
	}

	float Input::GetMouseX()
	{
		return 0;
	}

	float Input::GetMouseY()
	{
		return 0;
	}

}