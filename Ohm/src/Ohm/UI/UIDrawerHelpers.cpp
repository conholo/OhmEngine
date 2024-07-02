#include "ohmpch.h"
#include "Ohm/UI/UIDrawerHelpers.h"
#include <imgui.h>
#include <imgui_internal.h>

namespace Ohm
{
	namespace UI
	{
		bool DrawVector2FieldTable(const std::string& label, glm::vec2& value, float resetValue)
		{
			bool updated = false;

			ImGui::PushID(label.c_str());

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
			if (ImGui::BeginTable("##Vec2Table", 3, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInner))
			{
				float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
				ImVec2 buttonSize = { lineHeight + 10.0f, lineHeight };

				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				if (ImGui::Button("X", buttonSize))
					value.x = resetValue;
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				updated |= ImGui::DragFloat("##X", &value.x, 0.01f, 0.0f, 0.0f, "%.2f");
				ImGui::PopItemWidth();

				ImGui::TableSetColumnIndex(1);
				if (ImGui::Button("Y", buttonSize))
					value.y = resetValue;
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				updated |= ImGui::DragFloat("##Y", &value.y, 0.01f, 0.0f, 0.0f, "%.2f");
				ImGui::PopItemWidth();

				ImGui::EndTable();
			}
			ImGui::PopStyleVar();

			ImGui::PopID();
			return updated;
		}

		bool DrawVector3Field(const std::string& label, glm::vec3& value, float resetValue)
		{
			bool updated = false;

			ImGui::PushID(label.c_str());

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.0f);
			ImGui::Text(label.c_str());
			ImGui::NextColumn();

			ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

			if (ImGui::Button("X", buttonSize))
				value.x = resetValue;

			ImGui::SameLine();
			updated |= ImGui::DragFloat("##X", &value.x, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();

			if (ImGui::Button("Y", buttonSize))
				value.y = resetValue;

			ImGui::SameLine();
			updated |= ImGui::DragFloat("##Y", &value.y, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();

			if (ImGui::Button("Z", buttonSize))
				value.z = resetValue;

			ImGui::SameLine();
			updated |= ImGui::DragFloat("##Z", &value.z, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();

			ImGui::PopStyleVar();
			ImGui::Columns(1);
			ImGui::PopID();
			
			return updated;
		}

		bool DrawVector3FieldTable(const std::string& label, glm::vec3& value, float resetValue)
		{
			bool updated = false;

			ImGui::PushID(label.c_str());

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
			if (ImGui::BeginTable("##Vec3Table", 3, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInner))
			{
				float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
				ImVec2 buttonSize = { lineHeight + 10.0f, lineHeight };

				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				if (ImGui::Button("X", buttonSize))
					value.x = resetValue;
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				updated |= ImGui::DragFloat("##X", &value.x, 0.01f, 0.0f, 0.0f, "%.2f");
				ImGui::PopItemWidth();

				ImGui::TableSetColumnIndex(1);
				if (ImGui::Button("Y", buttonSize))
					value.y = resetValue;
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				updated |= ImGui::DragFloat("##Y", &value.y, 0.01f, 0.0f, 0.0f, "%.2f");
				ImGui::PopItemWidth();

				ImGui::TableSetColumnIndex(2);
				if (ImGui::Button("Z", buttonSize))
					value.z = resetValue;
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				updated |= ImGui::DragFloat("##Z", &value.z, 0.01f, 0.0f, 0.0f, "%.2f");
				ImGui::PopItemWidth();

				ImGui::EndTable();
			}
			ImGui::PopStyleVar();

			ImGui::PopID();
			return updated;
		}

		bool DrawVector4Field(const std::string& label, glm::vec4& value, float resetValue /*= 1.0f*/)
		{
			bool updated = false;

			ImGui::PushID(label.c_str());

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
			if (ImGui::BeginTable("##SomeTable", 4, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInner))
			{
				float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
				ImVec2 buttonSize = { lineHeight + 10.0f, lineHeight };

				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 1.0f, 0.0f, 0.0f, 1.0f });
				if (ImGui::Button("R", buttonSize))
					value.r = resetValue;
				ImGui::PopStyleColor();
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				updated |= ImGui::DragFloat("##R", &value.r, 0.01f, 0.0f, 0.0f, "%.2f");
				ImGui::PopItemWidth();

				ImGui::TableSetColumnIndex(1);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.0f, 1.0f, 0.0f, 1.0f });
				if (ImGui::Button("G", buttonSize))
					value.g = resetValue;
				ImGui::PopStyleColor();
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				updated |= ImGui::DragFloat("##G", &value.g, 0.01f, 0.0f, 0.0f, "%.2f");
				ImGui::PopItemWidth();

				ImGui::TableSetColumnIndex(2);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.0f, 0.0f, 1.0f, 1.0f });
				if (ImGui::Button("B", buttonSize))
					value.b = resetValue;
				ImGui::PopStyleColor();
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				updated |= ImGui::DragFloat("##B", &value.b, 0.01f, 0.0f, 0.0f, "%.2f");
				ImGui::PopItemWidth();

				ImGui::TableSetColumnIndex(3);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.1f, 0.1f, 0.1f, 1.0f });
				if (ImGui::Button("A", buttonSize))
					value.b = resetValue;
				ImGui::PopStyleColor();
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				updated |= ImGui::DragFloat("##A", &value.a, 0.01f, 0.0f, 0.0f, "%.2f");
				ImGui::PopItemWidth();

				ImGui::EndTable();
			}
			ImGui::PopStyleVar();

			ImGui::PopID();

			return updated;
		}

		void HelpMarker(const char* desc)
		{
			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				ImGui::TextUnformatted(desc);
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}
		}

		void ErrorMarker(const char* desc)
		{
			ImGui::Text("ERROR!");
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				ImGui::TextUnformatted(desc);
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}
		}
	}
}