#pragma once

#include "IMesh.hpp"

#include "Poulpe/Core/IVisitor.hpp"

namespace Poulpe
{
    class Basic : public IVisitor, public IMesh
    {
    public:
        Basic(VulkanAdapter* adapter,
            TextureManager* textureManager,
            LightManager* lightManager);

        void visit(float const deltaTime, Mesh* mesh) override;
        void setPushConstants(Mesh* mesh) override;

    //private:
    //    void createBBoxEntity(Mesh* mesh);

    private:
        VulkanAdapter* m_Adapter;
        TextureManager* m_TextureManager;
        LightManager* m_LightManager;
    };
}
