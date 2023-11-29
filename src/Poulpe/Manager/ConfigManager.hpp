#pragma once
#include "IConfigManager.hpp"

namespace Poulpe
{
    class ConfigManager : IConfigManager
    {
    public:
        ConfigManager();
        virtual ~ConfigManager() = default;

        virtual nlohmann::json appConfig() override;
        virtual nlohmann::json texturesConfig() override;
        virtual nlohmann::json soundConfig() override;
        virtual nlohmann::json shaderConfig() override;
        virtual nlohmann::json entityConfig(const std::string& levelName) override;
        virtual std::vector<std::string> listLevels() override;
        virtual std::vector<std::string> listSkybox() override;

    private:
        nlohmann::json m_AppConfig{};
        nlohmann::json m_TexturesConfig{};
        nlohmann::json m_ShaderConfig{};
        nlohmann::json m_SoundConfig{};
        nlohmann::json m_EntityConfig{};

        std::mutex m_MutexRead;

        std::string const m_LevelPath{ "config/levels/" };
    };
}
