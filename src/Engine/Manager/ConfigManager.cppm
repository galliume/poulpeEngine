export module Engine.Managers.ConfigManager;

import std;

import Engine.Core.Json;

import Engine.Component.Mesh;

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
    void switchCamera();
    std::uint8_t getCameraIndex() const { return _camera_index; }

    json const& loadLevelData(std::string const & levelName);

    json const& appConfig() const { return _app_config; }
    json const& lvlConfig() const { return _lvl_config; }
    json const& shaderConfig() const { return _shader_config; }
    json const& soundConfig() const { return _sound_config; }
    json const&  texturesConfig() const { return _textures_config; }

    template<typename T>
    requires std::same_as<T, std::string> || std::same_as<T, std::uint32_t>
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
    std::uint8_t _camera_index{ 0 };

    json _app_config;
    json _entity_config;
    json _shader_config;
    json _sound_config;
    json _textures_config;
    json _lvl_config;
  };
}
