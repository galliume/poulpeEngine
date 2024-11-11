#pragma once

#include <chrono>
#include <cstdint>
#include <random>

namespace Poulpe
{
  using IDType = std::uint64_t;
  using ComponentTypeID = IDType;

  //Unique ID for Entity
  class GUIDGenerator
  {
  public:
    using ms = std::chrono::milliseconds;

    static IDType getGUID()
    {
      auto millis = static_cast<uint64_t>(std::chrono::duration_cast<ms>(
        std::chrono::time_point_cast<ms>(std::chrono::system_clock::now()).time_since_epoch()
      ).count());

      std::random_device rd;
      std::mt19937_64 gen(rd());
      std::uniform_int_distribution<uint64_t> dis(0, std::numeric_limits<uint64_t>::max());
      std::uint64_t rando_number = dis(gen);

      return millis + rando_number;
    }
  };

  //Unique ID for Archetype
  template<class T>
  class TypeIdGenerator
  {
  private:

    static IDType _count;

  public:
    template<class U>
    static IDType GetNewID()
    {
      static const IDType idCounter = _count++;
      return idCounter;
    }
  };

  template<class T> IDType TypeIdGenerator<T>::_count = 0;
}
