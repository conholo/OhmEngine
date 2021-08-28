#pragma once


namespace Ohm
{
	class Time
	{
	public:
		Time(float time = 0.0f)
			:m_Time(time) { }

		operator float() const { return m_Time; }

		float Seconds() const { return m_Time; }
		float Milliseconds() const { return m_Time * 1000.0f; }

	private:
		float m_Time = 0.0f;
	};
}