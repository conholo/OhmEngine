#pragma once

#include "Ohm/Core/Memory.h"
#include "glm/gtx/string_cast.hpp"

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

		template<typename... Args>
		static void PrintAssertMessage(std::string_view Prefix, Args&&... args);

		static void AddSink(const spdlog::sink_ptr& sinkPointer);

	private:
		static Ref<spdlog::logger> s_EngineLogger;
		static Ref<spdlog::logger> s_ClientLogger;

		static std::vector<spdlog::sink_ptr> s_Sinks;
	};
}

template<typename OStream>
OStream& operator<<(OStream& os, const glm::vec3& vec)
{
	return os << '(' << vec.x << ", " << vec.y << ", " << vec.z << ')';
}

template<typename OStream>
OStream& operator<<(OStream& os, const glm::vec4& vec)
{
	return os << '(' << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ')';
}

namespace Ohm
{
	template<typename... Args>
	void Log::PrintAssertMessage(std::string_view Prefix, Args&&... args)
	{
		GetEngineLogger()->error("{0}: {1}", Prefix, fmt::format(std::forward<Args>(args)...));
	}
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