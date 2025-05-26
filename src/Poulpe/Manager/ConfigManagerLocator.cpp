module;
#include <memory>

module Poulpe.Managers.ConfigManagerLocator;

namespace Poulpe
{
  ConfigManager* ConfigManagerLocator::get()
  {
    return _config_manager.get();
  }

  void ConfigManagerLocator::init()
  {
    _config_manager = std::make_unique<ConfigManager>();
  }
}
