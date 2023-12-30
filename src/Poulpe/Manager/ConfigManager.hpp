#pragma once
#include "IConfigManager.hpp"

namespace Poulpe
{
    class ConfigManager : public IConfigManager
    {
    public:
        ConfigManager();
        virtual ~ConfigManager() = default;

        virtual nlohmann::json appConfig() const override { return m_AppConfig; };
        virtual std::vector<std::string> listLevels() const override;
        virtual std::vector<std::string> listSkybox() const override;
        virtual nlohmann::json loadLevelData(std::string const & levelName) override;
        virtual nlohmann::json shaderConfig() const override { return m_ShaderConfig;  };
        virtual nlohmann::json soundConfig() const override { return m_SoundConfig ;};
        virtual nlohmann::json texturesConfig() const override { return m_TexturesConfig; };

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
