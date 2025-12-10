export module Engine.Managers.ConfigManagerLocator;

import std;

import Engine.Managers.ConfigManager;

namespace Poulpe
{
  export class ConfigManagerLocator
  {
  public:
    static ConfigManager * get();
    static void init(std::string const& root_path);

  private:
    static std::unique_ptr<ConfigManager> _config_manager;
  };
}
