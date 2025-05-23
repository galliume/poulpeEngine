module;
#include <memory>

module Poulpe.Core.ConfigManagerLocator;

namespace Poulpe
{
  ConfigManager* ConfigManagerLocator::get()
  {
    return _configManager.get();
  }

  void ConfigManagerLocator::init()
  {
    _configManager = std::make_unique<ConfigManager>();
  }
}
