#include "Ohm.h"
#include "Ohm/Core/EntryPoint.h"

namespace Ohm
{
	class OhmEditor : public Application
	{
	public:
		OhmEditor()
			:Application("Ohm Editor")
		{
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

