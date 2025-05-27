module;
#include <memory>

module Poulpe.Managers.ConfigManagerLocator;

import Poulpe.Managers;

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
