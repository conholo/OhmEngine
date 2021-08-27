#pragma once

#include "Ohm/Core/Layer.h"

namespace Ohm
{
	class LayerStack
	{
	public:
		~LayerStack();

		void PushLayer(Layer* layer);
		void PopLayer(Layer* layer);

		void PushOverlay(Layer* overlay);
		void PopOverlay(Layer* overlay);
		
		std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
		std::vector<Layer*>::iterator end() { return m_Layers.end(); }

	private:
		uint32_t m_LayerInsertIndex = 0;
		std::vector<Layer*> m_Layers;
	};
}