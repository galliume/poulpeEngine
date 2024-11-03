#pragma once

#include <nlohmann/json.hpp>

#include <mutex>

namespace Poulpe
{
    class ConfigManager
    {
    public:
        ConfigManager();
        ~ConfigManager()  = default;

        nlohmann::json appConfig() const  { return m_AppConfig; }
        std::vector<std::string> listLevels() const ;
        std::vector<std::string> listSkybox() const ;
        nlohmann::json loadLevelData(std::string const & levelName) ;
        nlohmann::json shaderConfig() const  { return m_ShaderConfig;  }
        nlohmann::json soundConfig() const  { return m_SoundConfig ;}
        nlohmann::json texturesConfig() const  { return m_TexturesConfig; }

        template<typename T>
        requires std::same_as<T, std::string> || std::same_as<T, unsigned int>
        void updateConfig(std::string const & configName, T const & value)
        {
         m_AppConfig[configName] = value;
        }

    private:
        std::string const m_LevelPath{ "config/levels/" };

        nlohmann::json m_AppConfig{};
        nlohmann::json m_EntityConfig{};
        nlohmann::json m_ShaderConfig{};
        nlohmann::json m_SoundConfig{};
        nlohmann::json m_TexturesConfig{};

        std::mutex m_MutexRead;
    };
}
