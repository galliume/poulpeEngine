#pragma once
#include "IConfigManager.h"

namespace Rbk
{
    class ConfigManager : IConfigManager
    {
    public:
        ConfigManager();

        virtual nlohmann::json AppConfig() override;
        virtual nlohmann::json TexturesConfig() override;
        virtual nlohmann::json SoundConfig() override;
        virtual nlohmann::json ShaderConfig() override;
        virtual nlohmann::json EntityConfig(const std::string& levelName) override;
        virtual std::vector<std::string> ListLevels() override;
        virtual std::vector<std::string> ListSkybox() override;

    private:
        nlohmann::json m_AppConfig;
        nlohmann::json m_TexturesConfig;
        nlohmann::json m_ShaderConfig;
        nlohmann::json m_SoundConfig;
        nlohmann::json m_EntityConfig;

        std::mutex m_MutexRead;
    };
}
