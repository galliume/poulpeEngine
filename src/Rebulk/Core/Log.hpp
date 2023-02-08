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
