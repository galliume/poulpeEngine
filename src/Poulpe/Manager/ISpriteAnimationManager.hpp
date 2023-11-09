#pragma once

namespace Poulpe
{
    class ISpriteAnimationManager
    {
    public:
        ISpriteAnimationManager() = default;
        ~ISpriteAnimationManager() = default;

        virtual void add(const std::string& name, std::vector<std::string> sprites) = 0;
        virtual std::vector<std::string> getSpritesByName(const std::string& name) = 0;
    };
}
