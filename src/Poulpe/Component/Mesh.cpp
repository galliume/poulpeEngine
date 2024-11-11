#include "Mesh.hpp"

namespace Poulpe
{
    void Mesh::addUbos(const std::vector<UniformBufferObject>& ubos)
    {
        _data._ubos.insert(_data._ubos.end(), ubos.begin(), ubos.end());
    }
}
