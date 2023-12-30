#pragma once

#include "IVisitable.hpp"

#include "Poulpe/Manager/ILightManager.hpp"
#include "Poulpe/Manager/ITextureManager.hpp"

#include "Poulpe/Renderer/IRenderer.hpp"
namespace Poulpe
{
    class IVisitor
    {
    public:
        virtual ~IVisitor() {};

        virtual void init(IRenderer* const adapter,
          ITextureManager* const textureManager,
          ILightManager* const lightManager) = 0;
        
        virtual void visit(float const deltaTime, IVisitable* const visitable) = 0;
    };
}
