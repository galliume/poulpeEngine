#include "Log.hpp"

namespace Rbk {

    std::shared_ptr<spdlog::logger> Log::s_Logger;

    void Log::Init()
    {
        spdlog::set_pattern("%^[%T] %n: %v%$");

        s_Logger = spdlog::stdout_color_mt("ReBulkan");
#ifdef RBK_DEBUG_BUILD
        s_Logger->set_level(spdlog::level::trace);
#else
        s_Logger->set_level(spdlog::level::debug);
#endif
    }
}