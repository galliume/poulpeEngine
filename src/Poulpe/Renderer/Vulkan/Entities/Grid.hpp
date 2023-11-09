#pragma once

#include <volk.h>
#include "Poulpe/Core/IVisitor.hpp"
#include "Poulpe/Renderer/Adapter/VulkanAdapter.hpp"
#include "Poulpe/Component/Mesh.hpp"
#include "Poulpe/Manager/EntityManager.hpp"
#include "Poulpe/Manager/ShaderManager.hpp"
#include "Poulpe/Manager/TextureManager.hpp"

namespace Poulpe
{
    class Grid : public IVisitor
    {
    struct pc
    {
        glm::vec4 point;
        glm::mat4 view;
    };
    public:
        Grid(
            std::shared_ptr<VulkanAdapter> adapter,
            std::shared_ptr<EntityManager> entityManager,
            std::shared_ptr<ShaderManager> shaderManager,
            std::shared_ptr<TextureManager> textureManager,
            VkDescriptorPool descriptorPool
        );
        void visit(std::shared_ptr<Entity> entity) override;

    private:
        std::shared_ptr<VulkanAdapter> m_Adapter;
        std::shared_ptr<EntityManager> m_EntityManager;
        std::shared_ptr<ShaderManager> m_ShaderManager;
        std::shared_ptr<TextureManager> m_TextureManager;
        VkDescriptorPool m_DescriptorPool;

    };
}
