#include "ohmpch.h"
#include "Ohm/Core/Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>



namespace Ohm
{
	Ref<spdlog::logger> Log::s_EngineLogger = nullptr;
	Ref<spdlog::logger> Log::s_ClientLogger = nullptr;

	std::vector<spdlog::sink_ptr> Log::s_Sinks;

	void Log::Init()
	{
		std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Ohm.log", true));

		logSinks[0]->set_pattern("%^[%T] %n: %v%$");
		logSinks[1]->set_pattern("[%T] [%l] %n: %v");

		s_EngineLogger = std::make_shared<spdlog::logger>("OHM", begin(logSinks), end(logSinks));
		spdlog::register_logger(s_EngineLogger);
		s_EngineLogger->set_level(spdlog::level::trace);
		s_EngineLogger->flush_on(spdlog::level::trace);

		s_ClientLogger = std::make_shared<spdlog::logger>("APP", begin(logSinks), end(logSinks));
		spdlog::register_logger(s_ClientLogger);
		s_ClientLogger->set_level(spdlog::level::trace);
		s_ClientLogger->flush_on(spdlog::level::trace);
	}

	void Log::AddSink(const spdlog::sink_ptr& sinkPointer)
	{
		s_Sinks.push_back(sinkPointer);
		s_EngineLogger->sinks().push_back(sinkPointer);
		s_ClientLogger->sinks().push_back(sinkPointer);
	}
}