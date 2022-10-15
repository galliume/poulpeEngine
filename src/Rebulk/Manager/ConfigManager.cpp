#include "rebulkpch.h"
#include "ConfigManager.h"
#include <filesystem>

namespace Rbk
{

    ConfigManager::ConfigManager()
    {
        try {
            std::ifstream f("config/textures.json");
            m_TexturesConfig = nlohmann::json::parse(f);
        }
        catch (std::exception& e) {
            Rbk::Log::GetLogger()->critical("Cannot read texture config : {}", e.what());
            m_TexturesConfig = {};
        }

        try {
            std::ifstream f("config/sounds.json");
            m_SoundConfig = nlohmann::json::parse(f);
        }
        catch (std::exception& e) {
            Rbk::Log::GetLogger()->critical("Cannot read sounds config : {}", e.what());
            m_SoundConfig = {};
        }
    }

    nlohmann::json ConfigManager::EntityConfig(const std::string& levelName)
    {
        try {
            std::string level = "config/" + levelName + ".json";
            std::ifstream f(level);
            m_EntityConfig = nlohmann::json::parse(f);
        }
        catch (std::exception& e) {
            Rbk::Log::GetLogger()->critical("Cannot read level {} config : {}", levelName, e.what());
            m_EntityConfig = {};
        }

        return m_EntityConfig;
    }

    nlohmann::json ConfigManager::TexturesConfig()
    {
        return m_TexturesConfig;
    }

    nlohmann::json ConfigManager::SoundConfig()
    {
        return m_SoundConfig;
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
