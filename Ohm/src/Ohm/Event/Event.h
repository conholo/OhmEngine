#pragma once

#include <string>
#include <sstream>

namespace Ohm
{

	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowMoved,
		KeyPressed, KeyReleased, 
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};


#define EVENT_CLASS_TYPE(type)	static EventType GetStaticType() { return EventType::type; }\
								EventType GetEventType() const override { return GetStaticType(); }\
								const char* GetName() const override { return #type; }

	class Event
	{
	public:
		virtual ~Event() = default;

		bool Handled = false;

		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual std::string ToString() const { return GetName(); }
	};

	class EventDispatcher
	{
	public:
		EventDispatcher(Event& event)
			:m_Event(event) { }

		template<typename T, typename F>
		bool Dispatch(const F& func)
		{
			if (T::GetStaticType() == m_Event.GetEventType())
			{
				m_Event.Handled |= func(static_cast<T&>(m_Event));
				return true;
			}

			return false;
		}

	private:

		Event& m_Event;
	};

	inline std::ostream& operator << (std::ostream& stream, const Event& e)
	{
		return stream << e.ToString();
	}
}