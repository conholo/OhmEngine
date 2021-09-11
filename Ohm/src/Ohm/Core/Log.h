#pragma once

#include "Ohm/Core/Memory.h"

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace Ohm
{
	class Log
	{
	public:
		static void Init();

		static Ref<spdlog::logger>& GetEngineLogger() { return s_EngineLogger; }
		static Ref<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

		static void AddSink(const spdlog::sink_ptr& sinkPointer);

	private:
		static Ref<spdlog::logger> s_EngineLogger;
		static Ref<spdlog::logger> s_ClientLogger;

		static std::vector<spdlog::sink_ptr> s_Sinks;
	};
}


#define OHM_CORE_TRACE(...)		::Ohm::Log::GetEngineLogger()->trace(__VA_ARGS__)
#define OHM_CORE_INFO(...)		::Ohm::Log::GetEngineLogger()->info(__VA_ARGS__)
#define OHM_CORE_WARN(...)		::Ohm::Log::GetEngineLogger()->warn(__VA_ARGS__)
#define OHM_CORE_ERROR(...)		::Ohm::Log::GetEngineLogger()->error(__VA_ARGS__)
#define OHM_CORE_CRITICAL(...)	::Ohm::Log::GetEngineLogger()->critical(__VA_ARGS__)

#define OHM_TRACE(...)			::Ohm::Log::GetClientLogger()->trace(__VA_ARGS__)
#define OHM_INFO(...)			::Ohm::Log::GetClientLogger()->info(__VA_ARGS__)
#define OHM_WARN(...)			::Ohm::Log::GetClientLogger()->warn(__VA_ARGS__)
#define OHM_ERROR(...)			::Ohm::Log::GetClientLogger()->error(__VA_ARGS__)
#define OHM_CRITICAL(...)		::Ohm::Log::GetClientLogger()->critical(__VA_ARGS__)