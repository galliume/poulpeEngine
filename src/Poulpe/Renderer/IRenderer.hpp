#pragma once

#include "Poulpe/Component/Entity.hpp"
#include "Poulpe/Component/Mesh.hpp"
#include "Poulpe/Component/Mesh2D.hpp"
#include "Poulpe/Component/Vertex.hpp"
#include "Poulpe/Component/Vertex2D.hpp"

#include "Poulpe/Core/Buffer.hpp"
#include "Poulpe/Core/MeshData.hpp"

#include "Poulpe/GUI/Window.hpp"

namespace Poulpe
{
    class IRenderer
    {
        virtual ~IRenderer() {};
        virtual void init(VulkanAdapter* adapter, TextureManager* textureManager, LightManager* lightManager) = 0;
        virtual void setPushConstants(Mesh* mesh) = 0;
        virtual void createDescriptorSet(Mesh* mesh) = 0;
    };
}
