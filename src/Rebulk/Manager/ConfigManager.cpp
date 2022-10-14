#include "rebulkpch.h"
#include "ConfigManager.h"

#include <fstream>


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
}
