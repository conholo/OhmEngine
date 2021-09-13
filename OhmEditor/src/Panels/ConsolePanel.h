#pragma once

#include <imgui/imgui.h>

#include "spdlog/details/null_mutex.h"
#include "spdlog/sinks/base_sink.h"
#include <mutex>

namespace Ohm
{
	enum class LoggerType { None = 0, Core, Client, Both };

	using LogCallbackFn = std::function<void(const char*, LoggerType)>;

	template<typename Mutex>
	class console_sink : public spdlog::sinks::base_sink <Mutex>
	{
	public:
		console_sink() = default;
		console_sink(const LogCallbackFn& callback)
		{
			m_Callback = callback;
		}

	protected:
		void sink_it_(const spdlog::details::log_msg& msg) override
		{
			spdlog::memory_buf_t formatted;
			spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
			auto result = fmt::to_string(formatted);
			LoggerType type = msg.logger_name == "OHM" ? LoggerType::Core : LoggerType::Client;
			m_Callback(result.c_str(), type);
		}

		void flush_() override { }

	private:
		LogCallbackFn m_Callback;
	};

	using console_sink_mt = console_sink<std::mutex>;
	using console_sink_st = console_sink<spdlog::details::null_mutex>;


	class ConsolePanel
	{
	public:

		ConsolePanel()
		{
			m_AutoScroll = true;
			Clear();

			auto consoleSink = std::make_shared<console_sink_mt>(OHM_BIND_FN(ConsolePanel::AddLog));

			Log::AddSink(consoleSink);
		}

		void Clear()
		{
			m_Buffer.clear();
			m_LineOffsets.clear();
			m_LineOffsets.push_back(0);
		}

		void AddLog(const char* format, LoggerType type,  ...)IM_FMTARGS(2)
		{
			if (m_SelectedLoggerIndex == (int)LoggerType::Both && (type == LoggerType::Client || type == LoggerType::Core) || m_SelectedLoggerIndex == (int) type)
			{
				int old_size = m_Buffer.size();
				va_list args;
				va_start(args, format);
				m_Buffer.appendfv(format, args);
				va_end(args);
				for (int new_size = m_Buffer.size(); old_size < new_size; old_size++)
					if (m_Buffer[old_size] == '\n')
						m_LineOffsets.push_back(old_size + 1);
			}
		}


		void Draw(const char* title, bool* p_open = NULL)
		{
			if (!ImGui::Begin(title, p_open))
			{
				ImGui::End();
				return;
			}

			// Options menu
			if (ImGui::BeginPopup("Options"))
			{
				ImGui::Checkbox("Auto-scroll", &m_AutoScroll);

				int selectedIndex = m_SelectedLoggerIndex;

				ImGui::RadioButton("None", &selectedIndex, 0);
				ImGui::RadioButton("Core Log", &selectedIndex, 1);
				ImGui::RadioButton("Client Log", &selectedIndex, 2);
				ImGui::RadioButton("Both", &selectedIndex, 3);

				if (selectedIndex != m_SelectedLoggerIndex)
				{
					Clear();
					m_SelectedLoggerIndex = selectedIndex;
				}

				ImGui::EndPopup();
			}

			// Main window
			if (ImGui::Button("Options"))
				ImGui::OpenPopup("Options");
			ImGui::SameLine();
			bool clear = ImGui::Button("Clear");
			ImGui::SameLine();
			bool copy = ImGui::Button("Copy");
			ImGui::SameLine();
			m_Filter.Draw("Filter", -100.0f);

			ImGui::Separator();
			ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

			if (clear)
				Clear();
			if (copy)
				ImGui::LogToClipboard();

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			const char* buf = m_Buffer.begin();
			const char* buf_end = m_Buffer.end();
			if (m_Filter.IsActive())
			{
				// In this example we don't use the clipper when Filter is enabled.
				// This is because we don't have a random access on the result on our filter.
				// A real application processing logs with ten of thousands of entries may want to store the result of
				// search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
				for (int line_no = 0; line_no < m_LineOffsets.Size; line_no++)
				{
					const char* line_start = buf + m_LineOffsets[line_no];
					const char* line_end = (line_no + 1 < m_LineOffsets.Size) ? (buf + m_LineOffsets[line_no + 1] - 1) : buf_end;
					if (m_Filter.PassFilter(line_start, line_end))
						ImGui::TextUnformatted(line_start, line_end);
				}
			}
			else
			{
				// The simplest and easy way to display the entire buffer:
				//   ImGui::TextUnformatted(buf_begin, buf_end);
				// And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
				// to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
				// within the visible area.
				// If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
				// on your side is recommended. Using ImGuiListClipper requires
				// - A) random access into your data
				// - B) items all being the  same height,
				// both of which we can handle since we an array pointing to the beginning of each line of text.
				// When using the filter (in the block of code above) we don't have random access into the data to display
				// anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
				// it possible (and would be recommended if you want to search through tens of thousands of entries).
				ImGuiListClipper clipper;
				clipper.Begin(m_LineOffsets.Size);
				while (clipper.Step())
				{
					for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
					{
						const char* line_start = buf + m_LineOffsets[line_no];
						const char* line_end = (line_no + 1 < m_LineOffsets.Size) ? (buf + m_LineOffsets[line_no + 1] - 1) : buf_end;
						ImGui::TextUnformatted(line_start, line_end);
					}
				}
				clipper.End();
			}
			ImGui::PopStyleVar();

			if (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
				ImGui::SetScrollHereY(1.0f);

			ImGui::EndChild();
			ImGui::End();
		}

	private:
		int m_SelectedLoggerIndex = 3;
		ImGuiTextBuffer m_Buffer;
		ImGuiTextFilter m_Filter;
		ImVector<int> m_LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
		bool m_AutoScroll;  
		std::unordered_map<std::string, LoggerType> m_LoggerListings;
	};
}