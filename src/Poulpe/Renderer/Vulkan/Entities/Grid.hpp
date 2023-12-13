#pragma once

#include "IMesh.hpp"

#include "Poulpe/Core/IVisitor.hpp"

namespace Poulpe
{
    class Grid : public IVisitor, public IMesh
    {
    public:
        Grid(VulkanAdapter* adapter, TextureManager* textureManager);
        ~Grid() = default;

        void visit(float const deltaTime, Mesh* mesh) override;
        void setPushConstants(Mesh* mesh) override;
        void createDescriptorSet(Mesh* mesh) override;

    private:
        VulkanAdapter* m_Adapter;
        TextureManager* m_TextureManager;
    };
}
