#include "SpriteAnimationManager.hpp"

namespace Poulpe
{
    SpriteAnimationManager::SpriteAnimationManager()
    {

    }

    void SpriteAnimationManager::add(const std::string& name, std::vector<std::string> sprites)
    {
        m_SpriteAnimations[name] = sprites;
    }

    std::vector<std::string> SpriteAnimationManager::getSpritesByName(const std::string& name)
    {
        auto sprites = m_SpriteAnimations.find(name);

        if (sprites != m_SpriteAnimations.end()) {
            return sprites->second;
        } else {
            PLP_WARN("Sprites {} not found", name);
            return {};
        }
    }

    void SpriteAnimationManager::clear()
    {
        m_SpriteAnimations.clear();
    }
}
