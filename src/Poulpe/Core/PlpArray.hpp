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
      return _data[index];
    }
    T const& operator[](int index)
    {
      static_assert(S < index);
      return _data[index];
    }
    T* data() { return _data; }
    const T const * data() const { return _data; }

  private:
    T _data[S];
  };
}