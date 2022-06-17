#include "rebulkpch.h"
#include "EntityManager.h"

namespace Rbk
{
    EntityManager::EntityManager(std::shared_ptr<VulkanRenderer> renderer) : m_Renderer(renderer)
    {

    }

     uint32_t EntityManager::GetVerticesCount()
    {
        uint32_t total = 0;

       /* for (std::shared_ptr<Mesh> mesh : m_Meshes) {
            total += mesh.get()->vertices.size();
        }*/

        return total;
    }

    uint32_t EntityManager::GetIndicesCount()
    {
        uint32_t total = 0;

        //for (std::shared_ptr<Mesh>mesh : m_Meshes) {
        //    total += mesh.get()->indices.size();
        //}

        return total;
    }

    uint32_t EntityManager::GetInstancedCount()
    {
        uint32_t total = 0;

        //for (std::shared_ptr<Mesh> mesh : m_Meshes) {
        //    total += mesh.get()->ubos.size();
        //}

        return total;
    }

    void EntityManager::AddEntity(std::shared_ptr<Entity> entity)
    {
        m_Entities.emplace_back(entity);
    }
}