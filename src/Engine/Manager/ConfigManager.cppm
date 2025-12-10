module;
#include <nlohmann/json.hpp>

export module Engine.Managers.ConfigManager;

import std;

//import Engine.Core.Logger;

namespace Poulpe
{
  export class ConfigManager
  {
  public:
    ConfigManager(std::string const& root_path);
    ~ConfigManager() = default;

    void load();
    bool normalDebug();
    std::vector<std::string> listLevels();
    std::vector<std::string> listSkybox();
    bool reload();
    bool reloadShaders();
    void setNormalDebug();
    void setReload(bool const reload);
    void setReloadShaders(bool const reload);

    nlohmann::json const& loadLevelData(std::string const & levelName);

    nlohmann::json const& appConfig() const { return _app_config; }
    nlohmann::json const& lvlConfig() const { return _lvl_config; }
    nlohmann::json const& shaderConfig() const { return _shader_config; }
    nlohmann::json const& soundConfig() const { return _sound_config; }
    nlohmann::json const&  texturesConfig() const { return _textures_config; }

    template<typename T>
    requires std::same_as<T, std::string> || std::same_as<T, uint32_t>
    void updateConfig(std::string const & config_name, T const & value)
    {
      _app_config[config_name] = value;
    }

    std::string const& rootPath() const { return _root_path; }

  private:
    std::string const _root_path{};
    std::string const _level_path{};
    bool _reload_shaders{ false };
    bool _normal_debug{ false };
    bool _reload{ false };

    nlohmann::json _app_config;
    nlohmann::json _entity_config;
    nlohmann::json _shader_config;
    nlohmann::json _sound_config;
    nlohmann::json _textures_config;
    nlohmann::json _lvl_config;
  };
}
