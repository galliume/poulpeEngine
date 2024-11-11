#pragma once

#include "PlpTypedef.hpp"

namespace Poulpe
{
  class AssimpLoader

  {
  public:
    void static loadData(
      std::string const & path,
      bool const inverse_texture_y,
      std::function<void(
        PlpMeshData const _data,
        std::vector<material_t> const materials,
        bool const exists,
        std::vector<Animation> const animations,
        std::vector<Position> const positions,
        std::vector<Rotation> const rotations,
        std::vector<Scale> const scales)> callback);

  private:
     static std::string const cleanName(std::string const& name);
  };
}
