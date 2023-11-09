#pragma once
#include "ISpriteAnimationManager.hpp"

namespace Poulpe
{
    class SpriteAnimationManager : ISpriteAnimationManager
    {
    public:
        explicit SpriteAnimationManager();
        virtual ~SpriteAnimationManager() = default;

        virtual void add(const std::string& name, std::vector<std::string> sprites) override;
        virtual std::vector<std::string> getSpritesByName(const std::string& name) override;
        void clear();

    private:
        std::unordered_map<std::string, std::vector<std::string>> m_SpriteAnimations;
    };
}
