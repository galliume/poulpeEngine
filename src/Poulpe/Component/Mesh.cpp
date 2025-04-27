#include "Mesh.hpp"

namespace Poulpe
{
  void Mesh::addUbos(std::vector<std::vector<UniformBufferObject>> const& ubos)
  {
    _data._ubos.insert(_data._ubos.end(), ubos.begin(), ubos.end());
  }
}
