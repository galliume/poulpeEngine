export module Engine.Utils.ScopedTimer;

import std;

namespace Poulpe
{
  export class ScopedTimer
  {
  public:
    using ClockType = std::chrono::steady_clock;

    ScopedTimer(const char* func)
      : _function_name(func), _start(ClockType::now()) {}

    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer(ScopedTimer&&) = delete;

    auto operator=(const ScopedTimer&)->ScopedTimer & = delete;
    auto operator=(ScopedTimer&)->ScopedTimer & = delete;

    ~ScopedTimer()
    {
      auto stop = ClockType::now();
      auto duration = (stop - _start);
      auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

      std::cout << ms << " ms " << _function_name << "\n";
    }

  private:
    const char* _function_name{};
    const ClockType::time_point _start{};
  };

  #define USE_SCOPED_TIMER 1

  #if USE_SCOPED_TIMER
  #define SCOPED_TIMER() ScopedTimer timer{__func__}
  #else
  #define SCOPED_TIMER()
  #endif
}
