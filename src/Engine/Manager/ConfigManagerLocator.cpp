module;
#include <memory>

module Engine.Managers.ConfigManagerLocator;

import Engine.Managers.ConfigManager;

namespace Poulpe
{
  [[clang::no_destroy]] std::unique_ptr<ConfigManager> ConfigManagerLocator::_config_manager;

  ConfigManager* ConfigManagerLocator::get()
  {
    return _config_manager.get();
  }

  void ConfigManagerLocator::init()
  {
    _config_manager = std::make_unique<ConfigManager>();
  }
}
