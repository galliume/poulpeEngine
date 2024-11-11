#include "Mesh.hpp"

namespace Poulpe
{
    void Mesh::addUbos(const std::vector<UniformBufferObject>& ubos)
    {
        _Data._ubos.insert(_Data._ubos.end(), ubos.begin(), ubos.end());
    }
}
