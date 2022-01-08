#include "Log.h"
#include "rebulkpch.h"

namespace Rebulk {

	std::shared_ptr<spdlog::logger> Log::s_Logger;

	void Rebulk::Log::Init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");

		s_Logger = spdlog::stdout_color_mt("ReBulkan");
		s_Logger->set_level(spdlog::level::debug);
	}
}