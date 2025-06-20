module;
#include <memory>

export module Engine.Managers.ConfigManagerLocator;

import Engine.Managers.ConfigManager;

namespace Poulpe
{
  export class ConfigManagerLocator
  {
  public:
    static ConfigManager * get();
    static void init();

  private:
    static std::unique_ptr<ConfigManager> _config_manager;
  };
}
