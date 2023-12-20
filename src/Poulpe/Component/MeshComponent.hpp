#pragma once

#include "Component.hpp"

namespace Poulpe
{
    class MeshComponent : public Component, public IVisitor
    {
    public:
        MeshComponent() : Component() {};

        //@todo mmh ? CRTP ?
        void visit(float const deltaTime, Mesh* mesh) override {
            Component::visit(deltaTime, mesh);
        }

        Mesh* getMesh() { return m_Mesh.get(); }
        void setMesh(Mesh* mesh) { m_Mesh = std::unique_ptr<Mesh>(mesh); }

    private:
        std::unique_ptr<Mesh> m_Mesh = nullptr;
    };
}