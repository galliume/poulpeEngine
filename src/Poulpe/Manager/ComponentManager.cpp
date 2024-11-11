#include "ComponentManager.hpp"

namespace Poulpe
{
    void ComponentManager::clear()
    {
        _ComponentTypeMap.clear();
        _ComponentsEntityMap.clear();
    }
}
