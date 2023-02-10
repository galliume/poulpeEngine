#pragma once

namespace Rbk 
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

#ifdef RBK_DEBUG
    #define RBK_FATAL(...) ::Rbk::Log::GetLogger()->critical(__VA_ARGS__)
    #define RBK_ERROR(...) ::Rbk::Log::GetLogger()->error(__VA_ARGS__)
    #define RBK_WARN(...) ::Rbk::Log::GetLogger()->warn(__VA_ARGS__)
    #define RBK_INFO(...) ::Rbk::Log::GetLogger()->info(__VA_ARGS__)
    #define RBK_DEBUG(...) ::Rbk::Log::GetLogger()->debug(__VA_ARGS__)
    #define RBK_TRACE(...) ::Rbk::Log::GetLogger()->trace(__VA_ARGS__)
#else
    #define RBK_FATAL(...)
    #define RBK_ERROR(...)
    #define RBK_WARN(...)
    #define RBK_INFO(...)
    #define RBK_DEBUG(...) Rbk::Log::GetLogger()->debug(__VA_ARGS__)
    #define RBK_TRACE(...)
#endif