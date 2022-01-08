#pragma once
#include "rebulkpch.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Rebulk
{
	class Log
	{
	public:
		static void Init();
		inline static std::shared_ptr<spdlog::logger> GetLogger() { return s_Logger; }

	private:
		static std::shared_ptr<spdlog::logger> s_Logger;
	};
}
