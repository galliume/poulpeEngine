#pragma once

namespace Rbk
{
    class ISpriteAnimationManager
    {
    public:
        ISpriteAnimationManager() = default;
        ~ISpriteAnimationManager() = default;

        virtual void Add(const std::string& name, std::vector<std::string> sprites) = 0;
        virtual std::vector<std::string> GetSpritesByName(const std::string& name) = 0;
    };
}
