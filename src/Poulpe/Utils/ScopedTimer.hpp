#pragma once

#include <iostream>

class ScopedTimer
{
public:
  using ClockType = std::chrono::steady_clock;

  ScopedTimer(const char* func)
    : m_FunctionName(func), m_Start(ClockType::now()) {};

  ScopedTimer(const ScopedTimer&) = delete;
  ScopedTimer(ScopedTimer&&) = delete;

  auto operator=(const ScopedTimer&)->ScopedTimer & = delete;
  auto operator=(ScopedTimer&)->ScopedTimer & = delete;

  ~ScopedTimer()
  {
    auto stop = ClockType::now();
    auto duration = (stop - m_Start);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    
    std::cout << ms << " ms " << m_FunctionName << "\n";
  }

private:
  const char* m_FunctionName{};
  const ClockType::time_point m_Start{};
};

#define USE_SCOPED_TIMER 1

#if USE_SCOPED_TIMER
#define SCOPED_TIMER() ScopedTimer timer{__func__}
#else
#define SCOPED_TIMER()
#endif
