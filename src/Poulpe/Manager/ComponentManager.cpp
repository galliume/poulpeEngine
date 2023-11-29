#include "ComponentManager.hpp"

namespace Poulpe
{
    void ComponentManager::clear()
    {
        m_ComponentTypeMap.clear();
        m_ComponentsEntityMap.clear();
    }
}
