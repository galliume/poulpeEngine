#pragma once
#include <nlohmann/json.hpp>

namespace Poulpe
{
    class IConfigManager
    {
    public:
        IConfigManager() = default;
        ~IConfigManager() = default;

        virtual nlohmann::json appConfig() const = 0;
        virtual std::vector<std::string> listLevels() const = 0;
        virtual std::vector<std::string> listSkybox() const = 0;
        virtual nlohmann::json loadLevelData(std::string const & levelName) = 0;
        virtual nlohmann::json shaderConfig() const = 0;
        virtual nlohmann::json soundConfig() const = 0;
        virtual nlohmann::json texturesConfig() const = 0;
    };
}
