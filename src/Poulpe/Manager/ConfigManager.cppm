module;

#include <nlohmann/json.hpp>

#include <concepts>
#include <mutex>
#include <string>
#include <vector>

export module Poulpe.Manager.ConfigManager;

//import Poulpe.Core.Logger;

namespace Poulpe
{
  export class ConfigManager
  {
  public:
    ConfigManager();
    ~ConfigManager() = default;

    void load();
    double getElapsedTime();
    bool normalDebug();
    std::vector<std::string> listLevels();
    std::vector<std::string> listSkybox();
    bool reload();
    bool reloadShaders();
    void setElapsedTime(double const elapsed_time);
    void setNormalDebug();
    void setReload(bool const reload);
    void setReloadShaders(bool const reload);
    
    nlohmann::json appConfig();
    nlohmann::json loadLevelData(std::string const & levelName);
    nlohmann::json lvlConfig();
    nlohmann::json shaderConfig();
    nlohmann::json soundConfig();
    nlohmann::json texturesConfig();
 
    template<typename T>
    requires std::same_as<T, std::string> || std::same_as<T, unsigned int>
    void updateConfig(std::string const & config_name, T const & value)
    {
      _app_config[config_name] = value;
    }

  private:
    std::string const _levelPath{ "config/levels/" };
    bool _reload_shaders{ false };
    bool _normal_debug{ false };
    bool _reload{ false };
    double _elapsed_time{ 0.0 };

    nlohmann::json _app_config;
    nlohmann::json _entity_config;
    nlohmann::json _shader_config;
    nlohmann::json _sound_config;
    nlohmann::json _textures_config;
    nlohmann::json _lvl_config;
  };
}
