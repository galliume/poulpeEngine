#include "rebulkpch.h"
#include "EntityManager.h"

namespace Rbk
{
    EntityManager::EntityManager(std::shared_ptr<VulkanRenderer> renderer) : m_Renderer(renderer)
    {

    }

    uint32_t EntityManager::GetInstancedCount()
    {
        uint32_t total = 0;

        for (std::shared_ptr<Entity> entity : m_Entities) {
            std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(entity);

            if (mesh) total += 1;
        }

        return total;
    }

    void EntityManager::AddEntity(const std::shared_ptr<Entity> entity)
    {
        uint64_t count = m_LoadedEntities.count(entity->m_Name.c_str());

        if (0 != count) {
            std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(entity);
            
            if (mesh) {
                std::vector<Data> listData = *mesh->GetData();
                std::shared_ptr<Mesh> existingEntity = std::dynamic_pointer_cast<Mesh>(m_Entities[m_LoadedEntities[mesh->m_Name.c_str()][1]]);

                for (auto data : listData) {
                    existingEntity->AddUbos(data.m_Ubos);
                }

                m_LoadedEntities[mesh->m_Name][0] += 1;
            }
        } else {
            uint32_t index = m_Entities.size();

            m_LoadedEntities.insert({ entity->m_Name, { 1, index }});
            m_Entities.emplace_back(entity);
        }
    }

    std::shared_ptr<Entity> EntityManager::GetEntityByName(const std::string name)
    {
        std::vector<std::shared_ptr<Entity>>::iterator it = std::find_if(
            m_Entities.begin(),
            m_Entities.end(),
            [name](std::shared_ptr<Entity> entity) -> bool { return entity->m_Name == name; });

        return *it;
    }
}