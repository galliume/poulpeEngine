#pragma once
#include <nlohmann/json.hpp>

namespace Poulpe
{
    class IConfigManager
    {
    public:
        IConfigManager() = default;
        ~IConfigManager() = default;

        virtual nlohmann::json TexturesConfig() = 0;
        virtual nlohmann::json AppConfig() = 0;
        virtual nlohmann::json SoundConfig() = 0;
        virtual nlohmann::json ShaderConfig() = 0;
        virtual nlohmann::json EntityConfig(const std::string& levelName) = 0;
        virtual std::vector<std::string> ListLevels() = 0;
        virtual std::vector<std::string> ListSkybox() = 0;
    };
}
