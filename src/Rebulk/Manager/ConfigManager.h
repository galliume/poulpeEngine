#pragma once
#include "json.hpp"

namespace Rbk
{
    class ConfigManager
    {
    public:
        ConfigManager();
        nlohmann::json TexturesConfig();

    private:
        nlohmann::json m_TexturesConfig;
    };
}