#pragma once

namespace Poulpe 
{
    class Log 
    {
    public:
        static void init();
        inline static std::shared_ptr<spdlog::logger> getLogger() { return s_Logger; }

    private:
        static std::shared_ptr<spdlog::logger> s_Logger;
    };
}

//#ifdef PLP_DEBUG_BUILD
    #define PLP_FATAL(...) ::Poulpe::Log::getLogger()->critical(__VA_ARGS__)
    #define PLP_ERROR(...) ::Poulpe::Log::getLogger()->error(__VA_ARGS__)
    #define PLP_WARN(...) ::Poulpe::Log::getLogger()->warn(__VA_ARGS__)
    #define PLP_INFO(...) ::Poulpe::Log::getLogger()->info(__VA_ARGS__)
    #define PLP_DEBUG(...) ::Poulpe::Log::getLogger()->debug(__VA_ARGS__)
    #define PLP_TRACE(...) ::Poulpe::Log::getLogger()->trace(__VA_ARGS__)
//    #define PLP_FATAL(...)
//    #define PLP_ERROR(...)
//    #define PLP_WARN(...)
//    #define PLP_INFO(...)
//    #define PLP_DEBUG(...)
//    #define PLP_TRACE(...)
//#endif
