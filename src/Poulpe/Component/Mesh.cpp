#include "Mesh.hpp"

#include "Poulpe/Core/TinyObjLoader.hpp"

namespace Poulpe
{
    void Mesh::addUbos(const std::vector<UniformBufferObject>& ubos)
    {
        m_Data.m_Ubos.insert(m_Data.m_Ubos.end(), ubos.begin(), ubos.end());
    }
    
    uint32_t Mesh::getNextSpriteIndex()
    {
        uint32_t nextIndex = 0;

        if (m_SpritesIndex > 0 && m_SpritesIndex < m_SpritesCount) {
            nextIndex = m_SpritesIndex;
            m_SpritesIndex += 1;
        } else if (m_SpritesIndex == m_SpritesCount  || 0 == m_SpritesIndex) {
            m_SpritesIndex = 1;
        }

        return nextIndex;
    }
}
