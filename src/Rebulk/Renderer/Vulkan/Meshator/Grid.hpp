#pragma once

#include <volk.h>
#include "Rebulk/Core/IVisitor.hpp"
#include "Rebulk/Renderer/Adapter/VulkanAdapter.hpp"
#include "Rebulk/Component/Mesh.hpp"
#include "Rebulk/Manager/EntityManager.hpp"
#include "Rebulk/Manager/ShaderManager.hpp"
#include "Rebulk/Manager/TextureManager.hpp"

namespace Rbk
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
        void Visit(std::shared_ptr<Entity> entity) override;

    private:
        std::shared_ptr<VulkanAdapter> m_Adapter;
        std::shared_ptr<EntityManager> m_EntityManager;
        std::shared_ptr<ShaderManager> m_ShaderManager;
        std::shared_ptr<TextureManager> m_TextureManager;
        VkDescriptorPool m_DescriptorPool;

    };
}
