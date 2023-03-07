#include "ConfigManager.hpp"
#include <filesystem>

namespace Rbk
{

    ConfigManager::ConfigManager()
    {
        try {
            std::filesystem::path path{ "config/rebulkan.json" };
            std::ifstream f(std::filesystem::absolute(path), std::ios::binary);
            if (f.is_open()) m_AppConfig = nlohmann::json::parse(f);
        }
        catch (std::exception& e) {
            RBK_FATAL("Cannot read application config : {}", e.what());
            throw std::runtime_error("Can't read application configuration file");
        }

        try {
            std::filesystem::path path{ "config/textures.json" };
            std::ifstream f(std::filesystem::absolute(path), std::ios::binary);
            if (f.is_open()) m_TexturesConfig = nlohmann::json::parse(f);
        }
        catch (std::exception& e) {
            RBK_FATAL("Cannot read texture config : {}", e.what());
            throw std::runtime_error("Can't read textures configuration file");
        }

        try {
            std::filesystem::path path{ "config/sounds.json" };
            std::ifstream f(std::filesystem::absolute(path), std::ios::binary);
            if (f.is_open()) m_SoundConfig = nlohmann::json::parse(f);
        }
        catch (std::exception& e) {
            RBK_FATAL("Cannot read sounds config : {}", e.what());
            throw std::runtime_error("Can't read sounds configuration file");
        }

        try {
            std::filesystem::path path{ "config/shader.json" };
            std::ifstream f(std::filesystem::absolute(path), std::ios::binary);
            if (f.is_open()) m_ShaderConfig = nlohmann::json::parse(f);
        }
        catch (std::exception& e) {
            RBK_FATAL("Cannot read shader config : {}", e.what());
            throw std::runtime_error("Can't read shaders configuration file");
        }
    }

    nlohmann::json ConfigManager::EntityConfig(const std::string& levelName)
    {
        try {
            {
                std::lock_guard guard(m_MutexRead);
                std::filesystem::path level{ "config/" + levelName + ".json" };
                std::ifstream f(std::filesystem::absolute(level), std::ios::binary);
                if (f.is_open()) m_EntityConfig = nlohmann::json::parse(f);
            }
        }
        catch (std::exception& e) {
            RBK_FATAL("Cannot read level {} config : {}", levelName, e.what());
            m_EntityConfig = {};
        }

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
