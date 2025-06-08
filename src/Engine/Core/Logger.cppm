module;
#include <fmt/color.h>
#include <fmt/chrono.h>
#include <fmt/core.h>

export module Engine.Core.Logger;

namespace Poulpe
{
  export class Logger
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
