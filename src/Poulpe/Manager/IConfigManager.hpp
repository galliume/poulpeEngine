#pragma once
#include <nlohmann/json.hpp>

namespace Poulpe
{
    class IConfigManager
    {
    public:
        IConfigManager() = default;
        ~IConfigManager() = default;

        virtual nlohmann::json texturesConfig() = 0;
        virtual nlohmann::json appConfig() = 0;
        virtual nlohmann::json soundConfig() = 0;
        virtual nlohmann::json shaderConfig() = 0;
        virtual nlohmann::json entityConfig(const std::string& levelName) = 0;
        virtual std::vector<std::string> listLevels() = 0;
        virtual std::vector<std::string> listSkybox() = 0;
    };
}
