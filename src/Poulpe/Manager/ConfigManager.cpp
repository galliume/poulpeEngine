#include "ConfigManager.hpp"
#include <filesystem>

namespace fs = std::filesystem;

namespace Poulpe
{

    ConfigManager::ConfigManager()
    {
        fs::path path{};
        std::ifstream f;

        path = "config/poulpeEngine.json";
        f.open(fs::absolute(path));
        if (f.is_open()) m_AppConfig = nlohmann::json::parse(f);
        f.close();
  
        path = "config/sounds.json";
        f.open(fs::absolute(path));
        if (f.is_open()) m_SoundConfig = nlohmann::json::parse(f);
        f.close();

        path = "config/shaders.json";
        f.open(fs::absolute(path));
        if (f.is_open()) m_ShaderConfig = nlohmann::json::parse(f);
        f.close();
    }

    std::vector<std::string> ConfigManager::listLevels() const
    {
      std::vector<std::string> levels;

      auto entries = fs::directory_iterator(m_LevelPath);

      for (auto& entry : entries) {
        levels.emplace_back(entry.path().stem().string());
      }

      return levels;
    }

    std::vector<std::string> ConfigManager::listSkybox() const
    {
      std::vector<std::string> skybox;

      std::string path = "assets/texture/skybox/";
      auto entries = fs::directory_iterator(path);

      for (auto& entry : entries) {
        skybox.emplace_back(entry.path().stem().string());
      }

      return skybox;
    }

    nlohmann::json ConfigManager::loadLevelData(std::string const & levelName)
    {
        m_EntityConfig.clear();
        m_TexturesConfig.clear();

        fs::path path{};
        std::ifstream f;
        path = "config/textures.json";

        f.open(fs::absolute(path));
        if (f.is_open()) m_TexturesConfig = nlohmann::json::parse(f);
        f.close();

        fs::path level{ m_LevelPath + levelName + ".json" };
        try {
            f.open(fs::absolute(level), std::ios_base::in);
            if (f.is_open()) m_EntityConfig = nlohmann::json::parse(f);
        }
        catch (nlohmann::json::parse_error& ex) {
            PLP_ERROR("Parse error at byte {}", ex.byte);
        }

        f.close();

        for (auto& entities : m_EntityConfig["entities"].items()) {
            auto textures = entities.value();
            for (auto& [key, path] : textures["textures"].items()) {
                m_TexturesConfig["textures"][key] = static_cast<std::string>(path);
            }
        }
        return m_EntityConfig;
    }
}
