#pragma once

#include "IVisitable.hpp"

namespace Poulpe
{
    class VulkanAdapter;
    class TextureManager;
    class LightManager;

    class IVisitor
    {
    public:
        virtual ~IVisitor() {};
        virtual void visit(float const deltaTime, IVisitable* const visitable) = 0;
        virtual void init(VulkanAdapter* const adapter,
          TextureManager* const textureManager,
          LightManager* const lightManager) = 0;
    };
}
