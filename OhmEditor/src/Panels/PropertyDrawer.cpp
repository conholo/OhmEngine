#include "Panels/PropertyDrawer.h"
#include <imgui/imgui.h>
#include "Ohm/Rendering/BufferLayout.h"
#include "Ohm/Core/Input.h"
#include "Ohm/Core/Log.h"
#include <sstream>

namespace Ohm
{
	uint32_t UIProperty::s_UIDCounter = 0;

	UIPropertyType UIPropertyTypeFromShaderDataType(ShaderDataType shaderDataType, bool isColor)
	{
		switch(shaderDataType)
		{
			case ShaderDataType::Float:			return UIPropertyType::Float;
			case ShaderDataType::Float2:		return UIPropertyType::Vec2;
			case ShaderDataType::Float3:		return UIPropertyType::Vec3;
			case ShaderDataType::Float4:		return isColor ? UIPropertyType::Color : UIPropertyType::Vec4;
			case ShaderDataType::Int:			return UIPropertyType::Int;
			default:							return UIPropertyType::None;
		}
	}

	static void ClampFloat(float* value, float min, float max)
	{
		*value = *value < min ? min : (*value > max ? max : *value);
	}

	static void DrawFloatParametersPopup(uint32_t uuid, UIFloatParameters& floatParams)
	{
		std::stringstream ss;
		ss << "Float Properties" << "##" << uuid;

		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0) && Input::IsKeyPressed(Key::LeftShift))
			ImGui::OpenPopup(ss.str().c_str());

		if (ImGui::BeginPopup(ss.str().c_str()))
		{
			std::stringstream ssDrag;
			ssDrag << "Drag##" << uuid;
			ImGui::Checkbox(ssDrag.str().c_str(), &floatParams.IsDrag);

			std::stringstream ssMinValue;
			ssMinValue << "Min Value##" << uuid;
			ImGui::InputFloat(ssMinValue.str().c_str(), &floatParams.Min);

			std::stringstream ssMaxValue;
			ssMaxValue << "Max Value##" << uuid;
			ImGui::InputFloat(ssMaxValue.str().c_str(), &floatParams.Max);

			std::stringstream ssStep;
			ssStep << "Step##" << uuid;
			ImGui::InputFloat(ssStep.str().c_str(), &floatParams.SpeedStep);

			ImGui::EndPopup();
		}
	}

	void UIFloat::Draw()
	{
		ImGui::PushID(m_Label.c_str());

		std::stringstream ss;
		ss << "Float Properties" << "##" << m_UUID;

		if (m_FloatParameters.IsDrag)
		{
			std::stringstream ssDragFloat;
			ssDragFloat << m_Label.c_str() << "##Drag" << m_UUID;

			ImGui::DragFloat(ssDragFloat.str().c_str(), m_Value, m_FloatParameters.SpeedStep, m_FloatParameters.Min, m_FloatParameters.Max, m_FloatParameters.Format);
			DrawFloatParametersPopup(m_UUID, m_FloatParameters);
		}
		else
		{
			std::stringstream ssInputFloat;
			ssInputFloat << m_Label.c_str() << "##Input" << m_UUID;

			ImGui::InputFloat(ssInputFloat.str().c_str(), m_Value, m_FloatParameters.SpeedStep, m_FloatParameters.FastStep, m_FloatParameters.Format);
			DrawFloatParametersPopup(m_UUID, m_FloatParameters);
		}

		ClampFloat(m_Value, m_FloatParameters.Min, m_FloatParameters.Max);

		ImGui::PopID();
	}

	void UIInt::Draw()
	{
		ImGui::InputInt(m_Label.c_str(), m_Value, m_Step);
	}

	void UIVector2::Draw()
	{
		ImGui::PushID(m_Label.c_str());

		std::stringstream ss;
		ss << "Vector2 Properties" << "##" << m_UUID;

		if (m_FloatParameters.IsDrag)
		{
			std::stringstream ssDragFloat;
			ssDragFloat << m_Label.c_str() << "##Drag" << m_UUID;

			ImGui::DragFloat2(ssDragFloat.str().c_str(), &m_Value->x, m_FloatParameters.SpeedStep, m_FloatParameters.Min, m_FloatParameters.Max, m_FloatParameters.Format);
			DrawFloatParametersPopup(m_UUID, m_FloatParameters);
		}
		else
		{
			std::stringstream ssInputFloat;
			ssInputFloat << m_Label.c_str() << "##Input" << m_UUID;

			ImGui::InputFloat2(ssInputFloat.str().c_str(), &m_Value->x, m_FloatParameters.Format);
			DrawFloatParametersPopup(m_UUID, m_FloatParameters);
		}

		ClampFloat(&m_Value->x, m_FloatParameters.Min, m_FloatParameters.Max);
		ClampFloat(&m_Value->y, m_FloatParameters.Min, m_FloatParameters.Max);

		ImGui::PopID();
	}

	void UIVector3::Draw()
	{
		ImGui::PushID(m_Label.c_str());

		std::stringstream ss;
		ss << "Vector3 Properties" << "##" << m_UUID;

		if (m_FloatParameters.IsDrag)
		{
			std::stringstream ssDragFloat;
			ssDragFloat << m_Label.c_str() << "##Drag" << m_UUID;

			ImGui::DragFloat3(ssDragFloat.str().c_str(), &m_Value->x, m_FloatParameters.SpeedStep, m_FloatParameters.Min, m_FloatParameters.Max, m_FloatParameters.Format);
			DrawFloatParametersPopup(m_UUID, m_FloatParameters);
		}
		else
		{
			std::stringstream ssInputFloat;
			ssInputFloat << m_Label.c_str() << "##Input" << m_UUID;

			ImGui::InputFloat3(ssInputFloat.str().c_str(), &m_Value->x, m_FloatParameters.Format);
			DrawFloatParametersPopup(m_UUID, m_FloatParameters);
		}

		ClampFloat(&m_Value->x, m_FloatParameters.Min, m_FloatParameters.Max);
		ClampFloat(&m_Value->y, m_FloatParameters.Min, m_FloatParameters.Max);
		ClampFloat(&m_Value->z, m_FloatParameters.Min, m_FloatParameters.Max);

		ImGui::PopID();
	}

	void UIVector4::Draw()
	{
		ImGui::PushID(m_Label.c_str());

		std::stringstream ss;
		ss << "Vector4 Properties" << "##" << m_UUID;

		if (m_FloatParameters.IsDrag)
		{
			std::stringstream ssDragFloat;
			ssDragFloat << m_Label.c_str() << "##Drag" << m_UUID;

			ImGui::DragFloat4(ssDragFloat.str().c_str(), &m_Value->x, m_FloatParameters.SpeedStep, m_FloatParameters.Min, m_FloatParameters.Max, m_FloatParameters.Format);
			DrawFloatParametersPopup(m_UUID, m_FloatParameters);
		}
		else
		{
			std::stringstream ssInputFloat;
			ssInputFloat << m_Label.c_str() << "##Input" << m_UUID;

			ImGui::InputFloat4(ssInputFloat.str().c_str(), &m_Value->x, m_FloatParameters.Format);
			DrawFloatParametersPopup(m_UUID, m_FloatParameters);
		}

		ClampFloat(&m_Value->x, m_FloatParameters.Min, m_FloatParameters.Max);
		ClampFloat(&m_Value->y, m_FloatParameters.Min, m_FloatParameters.Max);
		ClampFloat(&m_Value->z, m_FloatParameters.Min, m_FloatParameters.Max);
		ClampFloat(&m_Value->w, m_FloatParameters.Min, m_FloatParameters.Max);

		ImGui::PopID();
	}

	void UIColor::Draw()
	{
		ImGui::PushID(m_Label.c_str());

		ImVec4 color{ m_Color->r, m_Color->g, m_Color->b, m_Color->a };

		std::stringstream ss;
		ss << "##" << m_UUID;

		ImGui::LabelText(ss.str().c_str(), m_Label.c_str());

		std::stringstream pickerSS;

		pickerSS << "\"" << m_Label.c_str() << "\"" << " Picker";

		if (ImGui::ColorButton(pickerSS.str().c_str(), color, ImGuiColorEditFlags_None, {50, 50}))
			ImGui::OpenPopup("Color Picker");

		if (ImGui::BeginPopup("Color Picker"))
		{
			ImGui::ColorPicker4(m_Label.c_str(), &m_Color->x, ImGuiColorEditFlags_None, NULL);
			ImGui::EndPopup();
		}

		ImGui::PopID();
	}
}