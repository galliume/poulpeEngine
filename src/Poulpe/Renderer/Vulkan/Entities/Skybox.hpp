#pragma once

#include "IMesh.hpp"

#include "Poulpe/Core/IVisitor.hpp"

namespace Poulpe
{
    class Skybox : public IVisitor, public IMesh
    {
    public:
        Skybox(VulkanAdapter* adapter);
        ~Skybox() = default;

        void visit(float const deltaTime, Mesh* mesh) override;
        void setPushConstants(Mesh* mesh) override;

    private:
        VulkanAdapter* m_Adapter;
    };
}
