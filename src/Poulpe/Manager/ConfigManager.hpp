#pragma once

#include <nlohmann/json.hpp>

#include <mutex>

namespace Poulpe
{
  class ConfigManager
  {
  public:
    ConfigManager();
    ~ConfigManager() = default;

    void load();
    nlohmann::json appConfig() const { return _app_config; }
    std::vector<std::string> listLevels() const ;
    std::vector<std::string> listSkybox() const ;
    nlohmann::json loadLevelData(std::string const & levelName) ;
    nlohmann::json shaderConfig() const { return _shader_config;  }
    nlohmann::json soundConfig() const { return _sound_config ;}
    nlohmann::json texturesConfig() const { return _textures_config; }

    void setNormalDebug() { _normal_debug = !_normal_debug; }
    void setReload() { _reload = !_reload; }
    void setReloadShaders(bool const reload) { _reload_shaders = reload; }

    bool normalDebug() const { return _normal_debug; }
    bool reload() const { return _reload; }
    bool reloadShaders() const { return _reload_shaders; }

    template<typename T>
    requires std::same_as<T, std::string> || std::same_as<T, unsigned int>
    void updateConfig(std::string const & configName, T const & value)
    {
      _app_config[configName] = value;
    }

  private:
    std::string const _LevelPath{ "config/levels/" };

    nlohmann::json _app_config{};
    nlohmann::json _entity_config{};
    nlohmann::json _shader_config{};
    nlohmann::json _sound_config{};
    nlohmann::json _textures_config{};

    bool _reload_shaders{ false };
    bool _normal_debug{ false };
    bool _reload{ false };
  };
}
