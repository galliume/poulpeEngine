module;
#include <memory>

export module Poulpe.Core.ConfigManagerLocator;

import Poulpe.Manager.ConfigManager;

export class ConfigManagerLocator
{
public:
  static ConfigManager* get() { return _configManager.get(); }

  static void init()
  {
    _configManager = std::make_unique<ConfigManager>();
  }

private:
  inline static std::unique_ptr<ConfigManager> _configManager;
};
