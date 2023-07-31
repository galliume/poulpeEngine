#include "ConfigManager.hpp"
#include <filesystem>

namespace Poulpe
{

    ConfigManager::ConfigManager()
    {
        std::filesystem::path path{};
        std::ifstream f;

        path = "config/poulpeEngine.json";
        f.open(std::filesystem::absolute(path));
        if (f.is_open()) m_AppConfig = nlohmann::json::parse(f);
        f.close();

        path = "config/textures.json";
        f.open(std::filesystem::absolute(path));
        if (f.is_open()) m_TexturesConfig = nlohmann::json::parse(f);
        f.close();
  
        path = "config/sounds.json";
        f.open(std::filesystem::absolute(path));
        if (f.is_open()) m_SoundConfig = nlohmann::json::parse(f);
        f.close();

        path = "config/shader.json";
        f.open(std::filesystem::absolute(path));
        if (f.is_open()) m_ShaderConfig = nlohmann::json::parse(f);
        f.close();
    }

    nlohmann::json ConfigManager::EntityConfig(const std::string& levelName)
    {
        std::filesystem::path level{ "config/" + levelName + ".json" };
        std::ifstream f;
        f.open(std::filesystem::absolute(level), std::ios_base::in);
        try {
            if (f.is_open()) m_EntityConfig = nlohmann::json::parse(f);
        }
        catch (nlohmann::json::parse_error& ex) {
            PLP_ERROR("Parse error at byte {}", ex.byte);
        }

        f.close();

        return m_EntityConfig;
    }

    nlohmann::json ConfigManager::AppConfig()
    {
        return m_AppConfig;
    }

    nlohmann::json ConfigManager::TexturesConfig()
    {
        return m_TexturesConfig;
    }

    nlohmann::json ConfigManager::SoundConfig()
    {
        return m_SoundConfig;
    }

    nlohmann::json ConfigManager::ShaderConfig()
    {
        return m_ShaderConfig;
    }

    std::vector<std::string> ConfigManager::ListLevels()
    {
        std::vector<std::string> levels;

        std::string path = "config/";
        auto entries = std::filesystem::directory_iterator(path);

        for (auto& entry : entries) {
            if (std::string::npos != entry.path().filename().string().find("level_")) {
                levels.emplace_back(entry.path().stem().string());
            }
        }

        return levels;
    }

    std::vector<std::string> ConfigManager::ListSkybox()
    {
        std::vector<std::string> skybox;

        std::string path = "assets/texture/skybox/";
        auto entries = std::filesystem::directory_iterator(path);

        for (auto& entry : entries) {
            skybox.emplace_back(entry.path().stem().string());
        }

        return skybox;
    }
}
