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
      return _Data[index];
    }
    T const& operator[](int index)
    {
      static_assert(S < index);
      return _Data[index];
    }
    T* data() { return _Data; }
    const T const * data() const { return _Data; }

  private:
    T _Data[S];
  };
}