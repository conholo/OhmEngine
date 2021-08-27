#include "Ohm.h"
#include "Ohm/Core/EntryPoint.h"

#include "EditorLayer.h"

namespace Ohm
{
	class OhmEditor : public Application
	{
	public:
		OhmEditor()
			:Application("Ohm Editor")
		{
			PushLayer(new EditorLayer());
		}

		~OhmEditor()
		{

		}
	};

	Application* CreateApplication()
	{
		return new OhmEditor;
	}
}

