module;
#include <memory>

export module Poulpe.Managers.ConfigManagerLocator;

import Poulpe.Managers;

namespace Poulpe
{
  export class ConfigManagerLocator
  {
  public:
    static ConfigManager* get();
    static void init();

  private:
    static std::unique_ptr<ConfigManager> _config_manager;
  };
}
