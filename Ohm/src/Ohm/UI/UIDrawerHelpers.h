#pragma once


namespace Ohm
{
	namespace UI
	{
		bool DrawVector2FieldTable(const std::string& label, glm::vec2& value, float resetValue = 0.0f);
		bool DrawVector3Field(const std::string& label, glm::vec3& value, float resetValue = 0.0f);
		bool DrawVector3FieldTable(const std::string& label, glm::vec3& value, float resetValue = 0.0f);
		bool DrawVector4Field(const std::string& label, glm::vec4& value, float resetValue = 1.0f);

		void HelpMarker(const char* desc);
		void ErrorMarker(const char* desc);
	}
}