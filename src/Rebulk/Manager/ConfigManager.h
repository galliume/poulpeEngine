#pragma once
#include "json.hpp"

namespace Rbk
{
    class ConfigManager
    {
    public:
        ConfigManager();

        nlohmann::json TexturesConfig();
        nlohmann::json SoundConfig();
        nlohmann::json EntityConfig(const std::string& levelName);
        std::vector<std::string> ListLevels();
        std::vector<std::string> ListSkybox();

    private:
        nlohmann::json m_TexturesConfig;
        nlohmann::json m_SoundConfig;
        nlohmann::json m_EntityConfig;
    };
}