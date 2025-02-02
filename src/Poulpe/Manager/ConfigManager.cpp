#include "ConfigManager.hpp"
#include <filesystem>

namespace fs = std::filesystem;

namespace Poulpe
{
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

  std::vector<std::string> ConfigManager::listLevels() const
  {
    std::vector<std::string> levels;

    auto entries = fs::directory_iterator(_LevelPath);

    std::ranges::for_each(entries, [&levels](auto& entry) {
      levels.emplace_back(entry.path().stem().string());
    });

    return levels;
  }

  std::vector<std::string> ConfigManager::listSkybox() const
  {
    std::vector<std::string> skybox;

    std::string path = "assets/texture/skybox/";
    auto entries = fs::directory_iterator(path);

    std::ranges::for_each(entries, [&skybox](auto& entry) {
      skybox.emplace_back(entry.path().stem().string());
    });

    return skybox;
  }

  nlohmann::json ConfigManager::loadLevelData(std::string const & levelName)
  {
    _entity_config.clear();
    _textures_config.clear();
    _lvl_config.clear();

    fs::path path{};
    std::ifstream f;
    path = "config/textures.json";

    f.open(fs::absolute(path));
    if (f.is_open()) _textures_config = nlohmann::json::parse(f);
    f.close();

    fs::path level{ _LevelPath + levelName + ".json" };
    try {
      f.open(fs::absolute(level), std::ios_base::in);
      if (f.is_open()) _entity_config = nlohmann::json::parse(f);
    }
    catch (nlohmann::json::parse_error& ex) {
      PLP_ERROR("Parse error at byte {}", ex.byte);
    }

    f.close();

    std::ranges::for_each(_entity_config["camera"].items(), [&](auto const& camera) {
      auto const& key = camera.key();
      auto const& data = camera.value();
      _lvl_config["camera"][key] = data;
    });

    std::ranges::for_each(_entity_config["entities"].items(), [&](auto const& entities) {
      auto const& textures = entities.value();
      
      if (textures.contains("textures")) {
        for (auto const& [key, data] : textures["textures"].items()) {
          _textures_config["textures"][key] = data;
        }
      }
      if (textures.contains("mr")) {
        for (auto const& [key, data] : textures["mr"].items()) {
          _textures_config["mr"][key] = data;
        }
      }
      if (textures.contains("normal")) {
        for (auto const& [key, data] : textures["normal"].items()) {
          _textures_config["normal"][key] = data;
        }
      }
      if (textures.contains("emissive")) {
        for (auto const& [key, data] : textures["emissive"].items()) {
          _textures_config["emissive"][key] = data;
        }
      }
      if (textures.contains("ao")) {
        for (auto const& [key, data] : textures["ao"].items()) {
          _textures_config["ao"][key] = data;
        }
      }
      if (textures.contains("transmission")) {
        for (auto const& [key, data] : textures["transmission"].items()) {
          _textures_config["transmission"][key] = data;
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
      _textures_config["water"][key] = data;
    });

    return _entity_config;
  }
}
