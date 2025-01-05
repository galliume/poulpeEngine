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

    std::ranges::for_each(_entity_config["entities"].items(), [&](auto const& entities) {
      auto textures = entities.value();
      for (auto const& [key, data] : textures["textures"].items()) {
        _textures_config["textures"][key] = data;
      }
      for (auto const& [key, data] : textures["orm"].items()) {
        _textures_config["orm"][key] = data;
      }
      for (auto const& [key, data] : textures["normal"].items()) {
        _textures_config["normal"][key] = data;
      }
    });
    return _entity_config;
  }
}
