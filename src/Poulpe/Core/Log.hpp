#pragma once

namespace Poulpe 
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

#ifdef PLP_DEBUG_BUILD
    #define PLP_FATAL(...) ::Poulpe::Log::GetLogger()->critical(__VA_ARGS__)
    #define PLP_ERROR(...) ::Poulpe::Log::GetLogger()->error(__VA_ARGS__)
    #define PLP_WARN(...) ::Poulpe::Log::GetLogger()->warn(__VA_ARGS__)
    #define PLP_INFO(...) ::Poulpe::Log::GetLogger()->info(__VA_ARGS__)
    #define PLP_DEBUG(...) ::Poulpe::Log::GetLogger()->debug(__VA_ARGS__)
    #define PLP_TRACE(...) ::Poulpe::Log::GetLogger()->trace(__VA_ARGS__)
#else
    #define PLP_FATAL(...)
    #define PLP_ERROR(...)
    #define PLP_WARN(...)
    #define PLP_INFO(...)
    #define PLP_DEBUG(...)
    #define PLP_TRACE(...)
#endif
