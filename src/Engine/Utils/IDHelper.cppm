export module Engine.Utils.IDHelper;

import std;

namespace Poulpe
{
  export using IDType = std::uint64_t;
  export using ComponentTypeID = IDType;

  inline std::atomic<IDType> _plp_component_type_global_counter{ 0 };

  //Unique ID for Entity
  export class GUIDGenerator
  {
  public:
    using ms = std::chrono::milliseconds;

    static IDType getGUID()
    {
      auto millis = static_cast<std::uint64_t>(std::chrono::duration_cast<ms>(
        std::chrono::time_point_cast<ms>(std::chrono::system_clock::now()).time_since_epoch()
      ).count());

      std::random_device rd;
      std::mt19937_64 gen(rd());
      std::uniform_int_distribution<std::uint64_t> dis(0, std::numeric_limits<std::uint64_t>::max());
      std::uint64_t rando_number = dis(gen);

      return millis + rando_number;
    }
  };

  export template<class T>
  class TypeIdGenerator
  {
  public:
    static IDType GetID()
    {
      static const IDType id = _plp_component_type_global_counter++;
      return id;
    }
  };
}
