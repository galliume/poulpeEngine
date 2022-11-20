#pragma once
#include "ISpriteAnimationManager.h"

namespace Rbk
{
    class SpriteAnimationManager : ISpriteAnimationManager
    {
    public:
        explicit SpriteAnimationManager();

        virtual void Add(const std::string& name, std::vector<std::string> sprites) override;
        virtual std::vector<std::string> GetSpritesByName(const std::string& name) override;
        void Clear();

    private:
        std::map<std::string, std::vector<std::string>> m_SpriteAnimations;
    };
}
