#pragma once

#include <iostream>

namespace Poulpe
{
  template<typename T, size_t S>
  class PlpArray
  {
  public:
    size_t size() const { return S; }
    T& operator[](int index)
    {
      static_assert(S < index);
      return m_Data[index];
    }
    T const& operator[](int index)
    {
      static_assert(S < index);
      return m_Data[index];
    }
    T* data() { return m_Data; }
    const T const * data() const { return m_Data; }

  private:
    T m_Data[S];
  };
}