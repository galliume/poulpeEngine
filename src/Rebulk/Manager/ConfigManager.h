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

    private:
        nlohmann::json m_TexturesConfig;
        nlohmann::json m_SoundConfig;
    };
}