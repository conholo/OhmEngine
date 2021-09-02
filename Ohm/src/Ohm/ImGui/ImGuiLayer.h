#pragma once

#include "Ohm/Core/Layer.h"

#include "Ohm/Event/WindowEvent.h"
#include "Ohm/Event/KeyEvent.h"
#include "Ohm/Event/MouseEvent.h"

namespace Ohm
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() = default;

		void OnAttach() override;
		void OnDetach() override;
		void OnEvent(Event& event) override;

		void Begin();
		void End();

		void SetDarkTheme();
	};
}