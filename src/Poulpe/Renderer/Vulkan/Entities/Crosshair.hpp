#pragma once

#include "IMesh.hpp"

#include "Poulpe/Core/IVisitor.hpp"

namespace Poulpe
{
    class Crosshair : public IVisitor, public IMesh
    {
    public:
        Crosshair(VulkanAdapter* adapter);

        void visit(float const deltaTime, Mesh* mesh) override;
        void setPushConstants(Mesh* mesh) override;

    private:
        VulkanAdapter* m_Adapter;
    };
}