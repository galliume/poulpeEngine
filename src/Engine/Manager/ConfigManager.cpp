module;
#include <nlohmann/json.hpp>

module Engine.Managers.ConfigManager;

import std;

import Engine.Core.Logger;

namespace Poulpe
{
  namespace fs = std::filesystem;

  ConfigManager::ConfigManager(std::string const& root_path)
  : _root_path(root_path)
  , _level_path(root_path + "/config/levels/")
  {
    load();
  }

  void ConfigManager::load()
  {
    fs::path path{};
    std::ifstream f;

    path = _root_path + "/config/poulpeEngine.json";

    f.open(fs::absolute(path));
    if (f.is_open()) _app_config = nlohmann::json::parse(f);
    f.close();

    path = _root_path + "/config/sounds.json";
    f.open(fs::absolute(path));
    if (f.is_open()) _sound_config = nlohmann::json::parse(f);
    f.close();

    path = _root_path + "/config/shaders.json";
    f.open(fs::absolute(path));
    if (f.is_open()) _shader_config = nlohmann::json::parse(f);
    f.close();
  }

  std::vector<std::string> ConfigManager::listLevels()
  {
    std::vector<std::string> levels;

    auto entries = fs::directory_iterator(_level_path);

    std::ranges::for_each(entries, [&levels](auto& entry) {
      levels.emplace_back(entry.path().stem().string());
    });

    return levels;
  }

  std::vector<std::string> ConfigManager::listSkybox()
  {
    std::vector<std::string> skybox;

    std::string path = _root_path + "/assets/texture/skybox/";
    auto entries = fs::directory_iterator(path);

    std::ranges::for_each(entries, [&skybox](auto& entry) {
      skybox.emplace_back(entry.path().stem().string());
    });

    return skybox;
  }

  nlohmann::json const& ConfigManager::loadLevelData(std::string const & levelName)
  {
    fs::path path{};
    std::ifstream f;
    path = _root_path + "/config/textures.json";

    f.open(fs::absolute(path));
    if (f.is_open()) _textures_config = nlohmann::json::parse(f);
    f.close();

    fs::path level{ _level_path + levelName + ".json" };
    try {
      f.open(fs::absolute(level), std::ios_base::in);
      if (f.is_open()) _entity_config = nlohmann::json::parse(f);
    }
    catch (nlohmann::json::parse_error& ex) {
      Logger::error("Parse error at byte {}", ex.byte);
    }

    f.close();

    std::ranges::for_each(_entity_config["camera"].items(), [&](auto const& camera) {
      auto const& key = camera.key();
      auto const& data = camera.value();
      _lvl_config["camera"][key] = data;
    });

    if (_entity_config.contains("hasFog")) {
      _lvl_config["hasFog"] = _entity_config.value("hasFog", false);
    }

    if (_entity_config.contains("player")) {
      _lvl_config["player"] = _entity_config.value("player", "");
    }

    std::ranges::for_each(_entity_config["terrain"].items(), [&](auto const& terrain) {
      auto const& key = terrain.key();
      auto const& data = terrain.value();
      _textures_config["terrain"][key] = data;
    });

    std::ranges::for_each(_entity_config["water"].items(), [&](auto const& water) {
      auto const& key = water.key();
      auto const& data = water.value();
      if (key == "_water_normal" || key == "_water_normal2") {
        _textures_config["normal"][key] = data;
      } else {
        _textures_config["water"][key] = data;
      }
    });

    return _entity_config;
  }

  void ConfigManager::setNormalDebug()
  {
    _normal_debug = !_normal_debug;
  }

  void ConfigManager::setReload(bool const reload)
  {
    _reload = reload;
  }

  void ConfigManager::setReloadShaders(bool const reload)
  {
    _reload_shaders = reload;
  }

  bool ConfigManager::normalDebug()
  {
    return _normal_debug;
  }

  bool ConfigManager::reload()
  {
    return _reload;
  }

  bool ConfigManager::reloadShaders()
  {
    return _reload_shaders;
  }
}
