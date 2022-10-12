#include "rebulkpch.h"
#include "ConfigManager.h"

#include <fstream>


namespace Rbk
{

    ConfigManager::ConfigManager()
    {
        nlohmann::json data;

        try {
            std::ifstream f("config/textures.json");
            m_TexturesConfig = nlohmann::json::parse(f);
        }
        catch (std::exception& e) {
            Rbk::Log::GetLogger()->critical("Cannot read texture config : {}", e.what());
            m_TexturesConfig = {};
        }
    }

    nlohmann::json ConfigManager::TexturesConfig()
    {
        return m_TexturesConfig;
    }
}
