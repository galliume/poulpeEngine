#pragma once

namespace Poulpe
{
    class Mesh;
    class VulkanAdapter;
    class TextureManager;
    class LightManager;

    class IVisitor
    {
    public:
        virtual ~IVisitor() {};
        virtual void visit(float const deltaTime, Mesh* mesh) = 0;
        virtual void init(VulkanAdapter* adapter, TextureManager* textureManager, LightManager* lightManager) = 0;
    };
}
