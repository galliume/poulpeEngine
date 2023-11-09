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

        path = "config/textures.json";
        f.open(fs::absolute(path));
        if (f.is_open()) m_TexturesConfig = nlohmann::json::parse(f);
        f.close();
  
        path = "config/sounds.json";
        f.open(fs::absolute(path));
        if (f.is_open()) m_SoundConfig = nlohmann::json::parse(f);
        f.close();

        path = "config/shader.json";
        f.open(fs::absolute(path));
        if (f.is_open()) m_ShaderConfig = nlohmann::json::parse(f);
        f.close();
    }

    nlohmann::json ConfigManager::entityConfig(const std::string& levelName)
    {
        fs::path level{ "config/" + levelName + ".json" };
        std::ifstream f;
        try {
            f.open(fs::absolute(level), std::ios_base::in);
            if (f.is_open()) m_EntityConfig = nlohmann::json::parse(f);
        }
        catch (nlohmann::json::parse_error& ex) {
            PLP_ERROR("Parse error at byte {}", ex.byte);
        }

        f.close();

        return m_EntityConfig;
    }

    nlohmann::json ConfigManager::appConfig()
    {
        return m_AppConfig;
    }

    nlohmann::json ConfigManager::texturesConfig()
    {
        return m_TexturesConfig;
    }

    nlohmann::json ConfigManager::soundConfig()
    {
        return m_SoundConfig;
    }

    nlohmann::json ConfigManager::shaderConfig()
    {
        return m_ShaderConfig;
    }

    std::vector<std::string> ConfigManager::listLevels()
    {
        std::vector<std::string> levels;

        std::string path = "config/";
        auto entries = fs::directory_iterator(path);

        for (auto& entry : entries) {
            if (std::string::npos != entry.path().filename().string().find("level_")) {
                levels.emplace_back(entry.path().stem().string());
            }
        }

        return levels;
    }

    std::vector<std::string> ConfigManager::listSkybox()
    {
        std::vector<std::string> skybox;

        std::string path = "assets/texture/skybox/";
        auto entries = fs::directory_iterator(path);

        for (auto& entry : entries) {
            skybox.emplace_back(entry.path().stem().string());
        }

        return skybox;
    }
}
