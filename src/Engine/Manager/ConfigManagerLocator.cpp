module Engine.Managers.ConfigManagerLocator;

import std;

import Engine.Managers.ConfigManager;

namespace Poulpe
{
  [[clang::no_destroy]] std::unique_ptr<ConfigManager> ConfigManagerLocator::_config_manager;

  ConfigManager* ConfigManagerLocator::get()
  {
    return _config_manager.get();
  }

  void ConfigManagerLocator::init(std::string const& root_path)
  {
    _config_manager = std::make_unique<ConfigManager>(root_path);
  }
}
