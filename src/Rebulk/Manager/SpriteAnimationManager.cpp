#include "rebulkpch.h"
#include "SpriteAnimationManager.h"

namespace Rbk
{
    SpriteAnimationManager::SpriteAnimationManager()
    {

    }

    void SpriteAnimationManager::Add(const std::string& name, std::vector<std::string> sprites)
    {
        m_SpriteAnimations[name] = sprites;
    }

    std::vector<std::string> SpriteAnimationManager::GetSpritesByName(const std::string& name)
    {
        auto sprites = m_SpriteAnimations.find(name);

        if (sprites != m_SpriteAnimations.end()) {
            return sprites->second;
        } else {
            Rbk::Log::GetLogger()->warn("Sprites {} not found", name);
            return {};
        }
    }

    void SpriteAnimationManager::Clear()
    {
        m_SpriteAnimations.clear();
    }
}
