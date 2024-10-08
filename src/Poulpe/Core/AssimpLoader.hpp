#pragma once

#include "PlpTypedef.hpp"

namespace Poulpe
{
  class AssimpLoader

  {
  public:
    void static loadData(
      std::string const & path,
      bool const shouldInverseTextureY,
      std::function<void(
        PlpMeshData const& _data,
         std::vector<material_t> const& materials,
         bool const exists)> callback);

  private:
     static std::string const cleanName(std::string const& name);
  };
}
