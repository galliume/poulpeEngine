module;
#include <memory>

module Poulpe.Core.ConfigManagerLocator;

namespace Poulpe
{
  std::unique_ptr<Poulpe::ConfigManager> Poulpe::ConfigManagerLocator::_config_manager;

  ConfigManager* ConfigManagerLocator::get()
  {
    return _config_manager.get();
  }

  void ConfigManagerLocator::init()
  {
    _config_manager = std::make_unique<ConfigManager>();
  }
}
