module Engine.Managers.ComponentManager;

import Engine.Component.Mesh;

namespace Poulpe
{
	void ComponentManager::clear()
	{
		_component_pools.clear();
	}

}
