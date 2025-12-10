module;
#include <nlohmann/json.hpp>

module Engine.Managers.ConfigManager;

import std;

import Engine.Core.Logger;

namespace Poulpe
{
  namespace fs = std::filesystem;

  ConfigManager::ConfigManager()
  {
    load();
  }

  void ConfigManager::load()
  {
    fs::path path{};
    std::ifstream f;

    path = "config/poulpeEngine.json";
    f.open(fs::absolute(path));
    if (f.is_open()) _app_config = nlohmann::json::parse(f);
    f.close();

    path = "config/sounds.json";
    f.open(fs::absolute(path));
    if (f.is_open()) _sound_config = nlohmann::json::parse(f);
    f.close();

    path = "config/shaders.json";
    f.open(fs::absolute(path));
    if (f.is_open()) _shader_config = nlohmann::json::parse(f);
    f.close();
  }

  std::vector<std::string> ConfigManager::listLevels()
  {
    std::vector<std::string> levels;

    auto entries = fs::directory_iterator(_levelPath);

    std::ranges::for_each(entries, [&levels](auto& entry) {
      levels.emplace_back(entry.path().stem().string());
    });

    return levels;
  }

  std::vector<std::string> ConfigManager::listSkybox()
  {
    std::vector<std::string> skybox;

    std::string path = "assets/texture/skybox/";
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
    path = "config/textures.json";

    f.open(fs::absolute(path));
    if (f.is_open()) _textures_config = nlohmann::json::parse(f);
    f.close();

    fs::path level{ _levelPath + levelName + ".json" };
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

    std::ranges::for_each(_entity_config["entities"].items(), [&](auto const& entities) {
      auto const& textures = entities.value();

      //@todo fix this ugly fix. Needs a real asset unique ID
      std::filesystem::path file_name{ textures["mesh"]};
      auto const& texture_prefix{ file_name.stem().string() + "_"};

      if (textures.contains("textures")) {
        for (auto const& [key, data] : textures["textures"].items()) {
          _textures_config["textures"][texture_prefix + key] = data;
        }
      }
      if (textures.contains("mr")) {
        for (auto const& [key, data] : textures["mr"].items()) {
          _textures_config["mr"][texture_prefix + key] = data;
        }
      }
      if (textures.contains("normal")) {
        for (auto const& [key, data] : textures["normal"].items()) {
          _textures_config["normal"][texture_prefix + key] = data;
        }
      }
      if (textures.contains("emissive")) {
        for (auto const& [key, data] : textures["emissive"].items()) {
          _textures_config["emissive"][texture_prefix + key] = data;
        }
      }
      if (textures.contains("ao")) {
        for (auto const& [key, data] : textures["ao"].items()) {
          _textures_config["ao"][texture_prefix + key] = data;
        }
      }
      if (textures.contains("transmission")) {
        for (auto const& [key, data] : textures["transmission"].items()) {
          _textures_config["transmission"][texture_prefix + key] = data;
        }
      }
    });

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
