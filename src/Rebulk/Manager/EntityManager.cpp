#include "rebulkpch.h"
#include "EntityManager.h"

namespace Rbk
{
    EntityManager::EntityManager(const std::shared_ptr<VulkanRenderer>& renderer) : m_Renderer(renderer)
    {

    }

    const uint32_t EntityManager::GetInstancedCount()
    {
        uint32_t total = 0;

        for (std::shared_ptr<Entity> entity : m_Entities) {
            std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(entity);

            if (mesh) total += 1;
        }

        return total;
    }

    void EntityManager::AddEntity(const std::shared_ptr<Entity>& entity)
    {
        uint64_t count = m_LoadedEntities.count(entity->GetName().c_str());

        if (0 != count) {
            std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(entity);
            
            if (mesh) {
                std::vector<Data> listData = *mesh->GetData();
                std::shared_ptr<Mesh> existingEntity = std::dynamic_pointer_cast<Mesh>(m_Entities[m_LoadedEntities[mesh->GetName().c_str()][1]]);

                for (auto data : listData) {
                    existingEntity->AddUbos(data.m_Ubos);
                }

                m_LoadedEntities[mesh->GetName()][0] += 1;
            }
        } else {
            uint32_t index = m_Entities.size();

            m_LoadedEntities.insert({ entity->GetName(), { 1, index }});
            m_Entities.emplace_back(entity);
        }
    }

    const std::shared_ptr<Entity> EntityManager::GetEntityByName(const std::string& name) const
    {
        auto it = std::find_if(
            m_Entities.cbegin(),
            m_Entities.cend(),
            [name](const std::shared_ptr<Entity>& entity) -> bool { return entity->GetName() == name; });

        return *it;
    }

    std::vector<std::future<void>> EntityManager::Load(nlohmann::json levelConfig)
    {
        std::vector<std::future<void>> futures;

       /* std::future<void> entitiesFuture = std::async(std::launch::async, [this, &levelConfig]() {*/
            Rbk::Log::GetLogger()->debug("{} entities", levelConfig["entities"].size());

            for (auto& entityConf : levelConfig["entities"].items()) {

                if ("deadTree" != entityConf.key()) continue;
                auto data = entityConf.value();
                std::shared_ptr<Rbk::Mesh> entity = std::make_shared<Rbk::Mesh>();

                Rbk::Log::GetLogger()->debug("entity {}", entityConf.key());

                std::string form = static_cast<std::string>(data["form"]);

                if ("square" == form) {

                    int xMin = static_cast<int>(data["squarePadding"]["x"][0]);
                    int xMax = static_cast<int>(data["squarePadding"]["x"][1]);
                    int yMin = static_cast<int>(data["squarePadding"]["y"][0]);
                    int yMax = static_cast<int>(data["squarePadding"]["y"][1]);

                    for (int x = xMin; x < xMax; x++) {
                        for (int y = yMin; y < yMax; y++) {

                            glm::vec3 position = glm::vec3(
                                static_cast<double>(data["positions"][0]["x"]) * static_cast<float>(x),
                                static_cast<double>(data["positions"][0]["y"]),
                                static_cast<double>(data["positions"][0]["z"]) * static_cast<float>(y)
                            );

                            std::vector<std::string> textures{};
                            for (auto& texture : data["textures"])
                                textures.emplace_back(static_cast<std::string>(texture));

                            entity->Init(
                                static_cast<std::string>(entityConf.key()),
                                static_cast<std::string>(data["mesh"]),
                                textures,
                                static_cast<std::string>(data["shader"]),
                                position,
                                glm::vec3(
                                    static_cast<double>(data["scales"][0]["x"]),
                                    static_cast<double>(data["scales"][0]["y"]),
                                    static_cast<double>(data["scales"][0]["z"])
                                ),
                                glm::vec3(
                                    static_cast<double>(data["rotations"][0]["x"]),
                                    static_cast<double>(data["rotations"][0]["y"]),
                                    static_cast<double>(data["rotations"][0]["z"])
                                ),
                                static_cast<bool>(data["inverseTextureY"])
                            );
                            AddEntity(entity);
                        }
                    }
                } else {
                    int count = static_cast<int>(data["count"]);
                    for (int i = 0; i < count; i++) {

                        glm::vec3 position{};

                        if ("positioned" == data["form"]) {
                            position = glm::vec3(
                                static_cast<double>(data["positions"][i]["x"]),
                                static_cast<double>(data["positions"][i]["y"]),
                                static_cast<double>(data["positions"][i]["z"])
                            );
                        }
                        else if ("line" == data["form"]) {
                            position = glm::vec3(
                                static_cast<double>(data["positions"][i]["x"]) + static_cast<double>(data["padding"]["x"]),
                                static_cast<double>(data["positions"][i]["y"]) + static_cast<double>(data["padding"]["y"]),
                                static_cast<double>(data["positions"][i]["z"]) + static_cast<double>(data["padding"]["y"])
                            );
                        }

                        std::vector<std::string> textures{};
                        for (auto& texture : data["textures"])
                            textures.emplace_back(static_cast<std::string>(texture));

                        entity->Init(
                            static_cast<std::string>(entityConf.key()),
                            static_cast<std::string>(data["mesh"]),
                            textures,
                            static_cast<std::string>(data["shader"]),
                            position,
                            glm::vec3(
                                static_cast<double>(data["scales"][i]["x"]),
                                static_cast<double>(data["scales"][i]["y"]),
                                static_cast<double>(data["scales"][i]["z"])
                            ),
                            glm::vec3(
                                static_cast<double>(data["rotations"][i]["x"]),
                                static_cast<double>(data["rotations"][i]["y"]),
                                static_cast<double>(data["rotations"][i]["z"])
                            ),
                            static_cast<bool>(data["inverseTextureY"])
                        );
                        AddEntity(entity);
                    }
                }
            }
        //});
        //
        //futures.emplace_back(std::move(entitiesFuture));

        return futures;
    }
}