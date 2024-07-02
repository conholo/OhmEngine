#include "ohmpch.h"

#include "Ohm/UI/PropertyDrawer.h"
#include "Ohm/Core/Input.h"
#include "Ohm/Core/Log.h"
#include "Ohm/Rendering//TextureLibrary.h"
#include "Ohm/UI/UIDrawerHelpers.h"

#include <imgui.h>

#include "imgui_internal.h"

namespace Ohm
{
	namespace UI
	{
		uint32_t UIProperty::s_UIDCounter = 0;

		UIPropertyType UIPropertyTypeFromShaderDataType(ShaderDataType ShaderDataType, bool isColor)
		{
			switch (ShaderDataType)
			{
				case ShaderDataType::Float:					return UIPropertyType::Float;
				case ShaderDataType::Float2:				return UIPropertyType::Vec2;
				case ShaderDataType::Float3:				return UIPropertyType::Vec3;
				case ShaderDataType::Float4:				return isColor ? UIPropertyType::Color : UIPropertyType::Vec4;
				case ShaderDataType::Int:					return UIPropertyType::Int;
				case ShaderDataType::Bool:					return UIPropertyType::Bool;
				case ShaderDataType::Sampler2D:				return UIPropertyType::Texture;
				default:									return UIPropertyType::None;
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

		bool UIFloat::Draw()
		{
			ImGui::PushID(m_Label.c_str());

			std::stringstream ss;
			ss << "Float Properties" << "##" << m_UUID;

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
			if (ImGui::BeginTable(m_Label.c_str(), 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::AlignTextToFramePadding();
				ImGui::Text(m_Label.c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				if (m_FloatParameters.IsDrag)
				{
					if(ImGui::DragFloat("", m_Value, m_FloatParameters.SpeedStep, m_FloatParameters.Min, m_FloatParameters.Max, m_FloatParameters.Format))
						return true;
				}
				else
				{
					if(ImGui::InputFloat("", m_Value, m_FloatParameters.SpeedStep, m_FloatParameters.FastStep, m_FloatParameters.Format))
						return true;
				}
				ImGui::PopItemWidth();
				DrawFloatParametersPopup(m_UUID, m_FloatParameters);
				ImGui::EndTable();
			}
			ImGui::PopStyleVar();

			ClampFloat(m_Value, m_FloatParameters.Min, m_FloatParameters.Max);

			ImGui::PopID();
			return false;
		}


		bool UIFloat::Draw(const std::string& label, float* value)
		{
			ImGui::PushID(label.c_str());

			std::stringstream ss;
			ss << "Float Properties" << "##" << label;

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
			if (ImGui::BeginTable(label.c_str(), 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::AlignTextToFramePadding();
				ImGui::Text(label.c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				if(ImGui::DragFloat("", value, 0.01f, 0.0f, 0.0f, "%.3f"))
					return true;
				ImGui::PopItemWidth();
				ImGui::EndTable();
			}
			ImGui::PopStyleVar();

			ImGui::PopID();
			return false;
		}

		bool UIFloat::DrawSlider(const std::string& label, float* value, float min, float max)
		{
			bool updated = false;
			ImGui::PushID(label.c_str());

			std::stringstream ss;
			ss << "Float Properties" << "##" << label;

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
			if (ImGui::BeginTable(label.c_str(), 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::AlignTextToFramePadding();
				ImGui::Text(label.c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				updated |= ImGui::SliderFloat("", value, min, max, "%.3f", ImGuiSliderFlags_AlwaysClamp);
				ImGui::PopItemWidth();
				ImGui::EndTable();
			}
			ImGui::PopStyleVar();

			ImGui::PopID();

			return updated;
		}

		bool UIFloat::DrawAngle(const std::string& label, float* rads, float min, float max)
		{
			bool updated = false;
			ImGui::PushID(label.c_str());

			std::stringstream ss;
			ss << "Float Properties" << "##" << label;

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
			if (ImGui::BeginTable(label.c_str(), 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::AlignTextToFramePadding();
				ImGui::Text(label.c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				updated |= ImGui::SliderAngle("", rads, min, max, "%.3f", ImGuiSliderFlags_AlwaysClamp);
				ImGui::PopItemWidth();
				ImGui::EndTable();
			}
			ImGui::PopStyleVar();

			ImGui::PopID();

			return updated;
		}
		
		bool UIInt::Draw()
		{
			return ImGui::InputInt(m_Label.c_str(), m_Value, m_Step);
		}

		bool UIInt::DrawDragInt(const std::string& label, int* value, float speed, int min, int max)
		{
			return ImGui::DragInt(label.c_str(), value, speed, min, max);
		}

		bool UIVector2::Draw()
		{
			bool updated = false;
			ImGui::PushID(m_Label.c_str());

			std::stringstream ss;
			ss << "Vector2 Properties" << "##" << m_UUID;

			if (m_FloatParameters.IsDrag)
			{
				std::stringstream ssDragFloat;
				ssDragFloat << m_Label.c_str() << "##Drag" << m_UUID;

				updated |= ImGui::DragFloat2(ssDragFloat.str().c_str(), &m_Value->x, m_FloatParameters.SpeedStep, m_FloatParameters.Min, m_FloatParameters.Max, m_FloatParameters.Format);
				DrawFloatParametersPopup(m_UUID, m_FloatParameters);
			}
			else
			{
				std::stringstream ssInputFloat;
				ssInputFloat << m_Label.c_str() << "##Input" << m_UUID;

				updated |= ImGui::InputFloat2(ssInputFloat.str().c_str(), &m_Value->x, m_FloatParameters.Format);
				DrawFloatParametersPopup(m_UUID, m_FloatParameters);
			}

			if (m_FloatParameters.Min < m_FloatParameters.Max && m_FloatParameters.Min != 0 && m_FloatParameters.Max != 0)
			{
				ClampFloat(&m_Value->x, m_FloatParameters.Min, m_FloatParameters.Max);
				ClampFloat(&m_Value->y, m_FloatParameters.Min, m_FloatParameters.Max);
			}

			ImGui::PopID();

			return updated;
		}

		bool UIVector2::Draw(const std::string& label, glm::vec2* value, bool readonly)
		{
			bool updated = false;

			ImGui::PushID(label.c_str());

			if(readonly)
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);

			std::stringstream ss;
			ss << "Vector2 Properties" << "##" << label;

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
			if (ImGui::BeginTable(label.c_str(), 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::AlignTextToFramePadding();
				ImGui::Text(label.c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				updated |= DrawVector2FieldTable("", *value, 1.0f);
				ImGui::PopItemWidth();
				ImGui::EndTable();
			}
			ImGui::PopStyleVar();

			if(readonly)
				ImGui::PopItemFlag();

			ImGui::PopID();
			return updated;
		}

		bool UIVector3::Draw()
		{
			bool updated = false;

			ImGui::PushID(m_Label.c_str());

			std::stringstream ss;
			ss << "Vector3 Properties" << "##" << m_UUID;

			if (m_FloatParameters.IsDrag)
			{
				std::stringstream ssDragFloat;
				ssDragFloat << m_Label.c_str() << "##Drag" << m_UUID;

				updated |= ImGui::DragFloat3(ssDragFloat.str().c_str(), &m_Value->x, m_FloatParameters.SpeedStep, m_FloatParameters.Min, m_FloatParameters.Max, m_FloatParameters.Format);
				DrawFloatParametersPopup(m_UUID, m_FloatParameters);
			}
			else
			{
				std::stringstream ssInputFloat;
				ssInputFloat << m_Label.c_str() << "##Input" << m_UUID;

				updated |= ImGui::InputFloat3(ssInputFloat.str().c_str(), &m_Value->x, m_FloatParameters.Format);
				DrawFloatParametersPopup(m_UUID, m_FloatParameters);
			}

			if (m_FloatParameters.Min < m_FloatParameters.Max && m_FloatParameters.Min != 0 && m_FloatParameters.Max != 0)
			{
				ClampFloat(&m_Value->x, m_FloatParameters.Min, m_FloatParameters.Max);
				ClampFloat(&m_Value->y, m_FloatParameters.Min, m_FloatParameters.Max);
				ClampFloat(&m_Value->z, m_FloatParameters.Min, m_FloatParameters.Max);
			}

			ImGui::PopID();
			return updated;
		}

		bool UIVector3::Draw(const std::string& label, glm::vec3* value, bool readonly)
		{
			bool updated = false;

			ImGui::PushID(label.c_str());

			if(readonly)
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);

			std::stringstream ss;
			ss << "Float Properties" << "##" << label;

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
			if (ImGui::BeginTable(label.c_str(), 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::AlignTextToFramePadding();
				ImGui::Text(label.c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				updated |= DrawVector3FieldTable("", *value, 1.0f);
				ImGui::PopItemWidth();
				ImGui::EndTable();
			}
			ImGui::PopStyleVar();

			if(readonly)
				ImGui::PopItemFlag();

			ImGui::PopID();
			return updated;
		}

		bool UIVector4::Draw()
		{
			bool updated = false;

			ImGui::PushID(m_Label.c_str());

			std::stringstream ss;
			ss << "Vector4 Properties" << "##" << m_UUID;

			if (m_FloatParameters.IsDrag)
			{
				std::stringstream ssDragFloat;
				ssDragFloat << m_Label.c_str() << "##Drag" << m_UUID;

				updated |= DrawVector4Field("", *m_Value, 1.0f);
				DrawFloatParametersPopup(m_UUID, m_FloatParameters);
			}
			else
			{
				std::stringstream ssInputFloat;
				ssInputFloat << m_Label.c_str() << "##Input" << m_UUID;

				updated |= ImGui::InputFloat4(ssInputFloat.str().c_str(), &m_Value->x, m_FloatParameters.Format);
				DrawFloatParametersPopup(m_UUID, m_FloatParameters);
			}

			if (m_FloatParameters.Min < m_FloatParameters.Max && m_FloatParameters.Min != 0 && m_FloatParameters.Max != 0)
			{
				ClampFloat(&m_Value->x, m_FloatParameters.Min, m_FloatParameters.Max);
				ClampFloat(&m_Value->y, m_FloatParameters.Min, m_FloatParameters.Max);
				ClampFloat(&m_Value->z, m_FloatParameters.Min, m_FloatParameters.Max);
				ClampFloat(&m_Value->w, m_FloatParameters.Min, m_FloatParameters.Max);
			}

			ImGui::PopID();

			return updated;
		}

		bool UIColor::Draw()
		{
			bool updated = false;
			ImGui::PushID(m_Label.c_str());

			ImVec4 color{ m_Color->r, m_Color->g, m_Color->b, m_Color->a };
			std::stringstream pickerSS;
			pickerSS << "\"" << m_Label.c_str() << "\"" << " Picker";

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
			if (ImGui::BeginTable(m_Label.c_str(), 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::AlignTextToFramePadding();
				ImGui::Text(m_Label.c_str());

				ImGui::TableSetColumnIndex(1);
				if (ImGui::ColorButton(pickerSS.str().c_str(), color, ImGuiColorEditFlags_HDR, { 10, 10 }))
					ImGui::OpenPopup("Color Picker");

				if (ImGui::BeginPopup("Color Picker"))
				{
					ImGuiColorEditFlags flags = ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float;
					updated |= ImGui::ColorPicker4(m_Label.c_str(), &m_Color->x, flags, nullptr);
					ImGui::EndPopup();
				}
				
				m_ColorVec4Drawer.Draw();

				ImGui::EndTable();
			}

			ImGui::PopStyleVar();
			ImGui::PopID();

			return updated;
		}

		bool UITexture2D::Draw()
		{
			bool updated = false;

			if (m_TextureUniform->HideInUI) return false;

			const Ref<Texture2D> CurrentTexture = TextureLibrary::Get2DFromID(m_TextureUniform->RendererID);
			if (CurrentTexture == nullptr) return false;

			ImGui::PushID(m_Label.c_str());

			std::stringstream ss;
			ss << "##" << m_UUID;

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
			if (ImGui::BeginTable(m_Label.c_str(), 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::AlignTextToFramePadding();
				ImGui::LabelText(ss.str().c_str(), m_Label.c_str());

				ImGui::TableSetColumnIndex(1);
				if (ImGui::ImageButton((ImTextureID)CurrentTexture->GetID(), { 50, 50 }, { 0, 1 }, { 1, 0 }))
					ImGui::OpenPopup("Texture Selection");

				if (ImGui::BeginPopup("Texture Selection"))
				{
					if (ImGui::BeginCombo("Texture Library", CurrentTexture->GetName().c_str()))
					{
						std::unordered_map<std::string, Ref<Texture2D>> availableTextures = TextureLibrary::Get2DLibrary();

						for (auto [name, texture] : availableTextures)
						{
							if (ImGui::Selectable(name.c_str(), true))
							{
								m_TextureUniform->RendererID = texture->GetID();
								updated = true;
							}
						}

						ImGui::EndCombo();
					}

					ImGui::EndPopup();
				}

				ImGui::EndTable();
			}

			ImGui::PopStyleVar();
			ImGui::PopID();

			return updated;
		}

		bool UIBool::Draw()
		{
			return ImGui::Checkbox(m_Label.c_str(), m_Value);
		}

		bool UIBool::Draw(const std::string& label, bool* value)
		{
			bool updated = false;
			ImGui::PushID(label.c_str());

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
			if (ImGui::BeginTable(label.c_str(), 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::AlignTextToFramePadding();
				ImGui::Text(label.c_str());
				ImGui::TableSetColumnIndex(1);
				updated |= ImGui::Checkbox("", value);
				ImGui::EndTable();
			}
			ImGui::PopStyleVar();

			ImGui::PopID();

			return updated;
		}
	}
}
