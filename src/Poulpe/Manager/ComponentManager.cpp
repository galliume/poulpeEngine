module Poulpe.Manager.ComponentManager;

namespace Poulpe
{
  void ComponentManager::clear()
  {
      _component_type_map.clear();
      _components_entity_map.clear();
  }
}
