#pragma once


#include "Ohm/Event/Event.h"

namespace Ohm
{
	class MouseButtonEvent : public Event
	{
	protected:
		MouseButtonEvent(int button)
			:m_Button(button) { }

		int GetButton() const { return m_Button; }

		int m_Button;
	};


	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(int button)
			:MouseButtonEvent(button) { }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << m_Button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed);
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(int button)
			:MouseButtonEvent(button) { }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_Button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased);
	};


	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float xOffset, float yOffset):
			m_XOffset(xOffset), m_YOffset(yOffset) { }

		float GetXOffset() const { return m_XOffset; }
		float GetYOffset() const { return m_YOffset; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrollEvent: " << "(" << m_XOffset << "," << m_YOffset << ")";
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled);

	private:
		float m_XOffset, m_YOffset;
	};


	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(float xPosition, float yPosition)
			:m_XPosition(xPosition), m_YPosition(yPosition) { }

		float GetXPosition() const { return m_XPosition; }
		float GetYPosition() const { return m_YPosition; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << "(" << m_XPosition << "," << m_YPosition << ")";
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved);

	private:
		float m_XPosition, m_YPosition;
	};
}