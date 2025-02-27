#pragma once

#include <fstream>

#include <fmt/color.h>
#include <fmt/chrono.h>
#include <fmt/core.h>

namespace Poulpe
{
  class Log
  {
  public:
    template <typename... Args>
    static void critical(fmt::format_string<Args...> fmt_str, Args&&... args)
    {
      fmt::print(
        fg(fmt::color::red) | fmt::emphasis::bold,
        fmt_str,
        std::forward<Args>(args)...);
      fmt::print("\n");
    }

    template <typename... Args>
    static void error(fmt::format_string<Args...> fmt_str, Args&&... args)
    {
      fmt::print(
        fg(fmt::color::red) | fmt::emphasis::bold,
        fmt_str,
        std::forward<Args>(args)...);
      fmt::print("\n");
    }

    template <typename... Args>
    static void warn(fmt::format_string<Args...> fmt_str, Args&&... args)
    {
      fmt::print(
        fg(fmt::color::orange),
        fmt_str,
        std::forward<Args>(args)...);
      fmt::print("\n");
    }

    template <typename... Args>
    static void info(fmt::format_string<Args...> fmt_str, Args&&... args)
    {
      fmt::print(
        fg(fmt::color::white),
        fmt_str,
        std::forward<Args>(args)...);
      fmt::print("\n");
    }

    template <typename... Args>
    static void debug(fmt::format_string<Args...> fmt_str, Args&&... args)
    {
      fmt::print(
        fg(fmt::color::green),
        fmt_str,
        std::forward<Args>(args)...);
      fmt::print("\n");
    }

    template <typename... Args>
    static void trace(fmt::format_string<Args...> fmt_str, Args&&... args)
    {
      fmt::print(
        fg(fmt::color::white),
        fmt_str,
        std::forward<Args>(args)...);
      fmt::print("\n");
    }
  };
}

//#ifdef PLP_DEBUG_BUILD
#define PLP_FATAL(...) ::Poulpe::Log::critical(__VA_ARGS__)
#define PLP_ERROR(...) ::Poulpe::Log::error(__VA_ARGS__)
#define PLP_WARN(...) ::Poulpe::Log::warn(__VA_ARGS__)
#define PLP_INFO(...) ::Poulpe::Log::info(__VA_ARGS__)
#define PLP_DEBUG(...) ::Poulpe::Log::debug(__VA_ARGS__)
#define PLP_TRACE(...) ::Poulpe::Log::trace(__VA_ARGS__)
//    #define PLP_FATAL(...)
//    #define PLP_ERROR(...)
//    #define PLP_WARN(...)
//    #define PLP_INFO(...)
//    #define PLP_DEBUG(...)
//    #define PLP_TRACE(...)
//#endif
