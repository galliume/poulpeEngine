#include "Mesh.hpp"

#include "Poulpe/Core/TinyObjLoader.hpp"

namespace Poulpe
{
    void Mesh::addUbos(const std::vector<UniformBufferObject>& ubos)
    {
        m_Data.m_Ubos.insert(m_Data.m_Ubos.end(), ubos.begin(), ubos.end());
    }

    void Mesh::visit(float const deltaTime, IVisitable* visitable)
    {
        //@todo visit impl
        PLP_WARN("visiting {} mesh {}", deltaTime, visitable->getName());
    }
}
