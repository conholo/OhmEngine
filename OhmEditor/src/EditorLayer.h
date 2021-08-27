#pragma once

#include "Ohm.h"
#include "Ohm/Rendering/EditorCamera.h"

namespace Ohm
{
	class EditorLayer : public Layer
	{
	public:

		EditorLayer();
		virtual ~EditorLayer();

		void OnAttach() override;
		void OnUpdate() override;
		void OnDetach() override;
		void OnUIRender() override;
		void OnEvent(Event& event) override;

	private:
		EditorCamera m_Camera;
	};
}