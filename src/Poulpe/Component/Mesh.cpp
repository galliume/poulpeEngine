#include "Mesh.hpp"

namespace Poulpe
{
    void Mesh::addUbos(const std::vector<UniformBufferObject>& ubos)
    {
        m_Data.m_Ubos.insert(m_Data.m_Ubos.end(), ubos.begin(), ubos.end());
    }

    void Mesh::visit(std::chrono::duration<float> deltaTime, IVisitable* visitable)
    {
        //@todo visit impl
        PLP_WARN("visiting {} mesh {}", deltaTime.count(), visitable->getName());
    }
}
