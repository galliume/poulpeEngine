#include "EntityManager.hpp"

namespace Rbk
{
    EntityManager::EntityManager() 
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
                Data* data = mesh->GetData();
                std::shared_ptr<Mesh> existingEntity = std::dynamic_pointer_cast<Mesh>(m_Entities[m_LoadedEntities[mesh->GetName().c_str()][1]]);
                existingEntity->AddUbos(data->m_Ubos);

                UniformBufferObject ubo;
                glm::mat4 transform = glm::translate(glm::mat4(1), mesh->GetBBox()->center) * glm::scale(glm::mat4(1), mesh->GetBBox()->size);
                ubo.model = mesh->GetBBox()->position * transform;
                existingEntity->GetBBox()->mesh->AddUbos({ ubo });

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

    std::function<void()> EntityManager::Load(nlohmann::json levelConfig, std::condition_variable& cv)
    {

        m_LevelConfig = levelConfig;

        std::function<void()> entitiesFuture = [=, this, &cv]() {

            for (auto& entityConf : m_LevelConfig["entities"].items()) {

                auto key = entityConf.key();
                auto data = entityConf.value();

                std::string form = static_cast<std::string>(data["form"]);

                if ("square" == form) {
                    int xMin = static_cast<int>(data["squarePadding"][0]["x"][0]);
                    int xMax = static_cast<int>(data["squarePadding"][0]["x"][1]);
                    int yMin = static_cast<int>(data["squarePadding"][0]["y"][0]);
                    int yMax = static_cast<int>(data["squarePadding"][0]["y"][1]);

                    for (int x = xMin; x < xMax; x++) {
                        for (int y = yMin; y < yMax; y++) {

                            std::shared_ptr<Rbk::Mesh> entity = std::make_shared<Rbk::Mesh>();
                            auto positionData = data["positions"].at(0);

                            glm::vec3 position = glm::vec3(
                                static_cast<float>(positionData["x"]) * static_cast<float>(x),
                                static_cast<float>(positionData["y"]),
                                static_cast<float>(positionData["z"]) * static_cast<float>(y)
                            );

                            std::vector<std::string> textures{};
                            for (auto& texture : data["textures"])
                                textures.emplace_back(static_cast<std::string>(texture));

                            auto scaleData = data["scales"].at(0);
                            auto rotationData = data["rotations"].at(0);

                            glm::vec3 scale = glm::vec3(
                                static_cast<float>(scaleData["x"]),
                                static_cast<float>(scaleData["y"]),
                                static_cast<float>(scaleData["z"])
                            );
                            glm::vec3  rotation = glm::vec3(
                                static_cast<float>(rotationData["x"]),
                                static_cast<float>(rotationData["y"]),
                                static_cast<float>(rotationData["z"])
                            );

                            bool hasBbox = static_cast<bool>(data["hasBbox"]);
                            entity->SetHasBbox(hasBbox);

                            auto entities = entity->Init(
                                static_cast<std::string>(key),
                                static_cast<std::string>(data["mesh"]),
                                textures,
                                static_cast<std::string>(data["shader"]),
                                position,
                                scale,
                                rotation,
                                static_cast<bool>(data["inverseTextureY"])
                            );

                            for (auto& e : entities) {
                                e->SetHasBbox(hasBbox);
                                AddEntity(e);
                            }
                        }
                    }
                } else {
                    int count = static_cast<int>(data["count"]);
                    std::string form = static_cast<std::string>(data["form"]);

                    for (int i = 0; i < count; i++) {

                        glm::vec3 position{};
                        auto positionData = (1 == data["positions"].size()) ? data["positions"].at(0) : data["positions"].at(i);

                        if ("positioned" == form) {
                            position = glm::vec3(
                                static_cast<float>(positionData["x"]),
                                static_cast<float>(positionData["y"]),
                                static_cast<float>(positionData["z"])
                            );
                        }
                        else if ("line" == form) {
                            position = glm::vec3(
                                static_cast<float>(positionData["x"]) + static_cast<float>(data["padding"]["x"]) * i,
                                static_cast<float>(positionData["y"]) + static_cast<float>(data["padding"]["y"]) * i,
                                static_cast<float>(positionData["z"]) + static_cast<float>(data["padding"]["z"]) * i
                            );
                        }

                        auto scaleData = (1 == data["scales"].size()) ? data["scales"].at(0) : data["scales"].at(i);
                        auto rotationData = (1 == data["rotations"].size()) ? data["rotations"].at(0) : data["rotations"].at(i);

                        glm::vec3 scale = glm::vec3(
                            static_cast<float>(scaleData["x"]),
                            static_cast<float>(scaleData["y"]),
                            static_cast<float>(scaleData["z"])
                        );
                        glm::vec3  rotation = glm::vec3(
                            static_cast<float>(rotationData["x"]),
                            static_cast<float>(rotationData["y"]),
                            static_cast<float>(rotationData["z"])
                        );

                        std::vector<std::string> textures{};
                        for (auto& texture : data["textures"])
                            textures.emplace_back(static_cast<std::string>(texture));

                        std::shared_ptr<Rbk::Mesh> entity = std::make_shared<Rbk::Mesh>();
                        bool hasBbox = static_cast<bool>(data["hasBbox"]);
                        entity->SetHasBbox(hasBbox);

                        auto entities = entity->Init(
                            static_cast<std::string>(key),
                            static_cast<std::string>(data["mesh"]),
                            textures,
                            static_cast<std::string>(data["shader"]),
                            position,
                            scale,
                            rotation,
                            static_cast<bool>(data["inverseTextureY"])
                        );
                        
                        for (auto& e : entities) {
                            e->SetHasBbox(hasBbox);
                            AddEntity(e);
                        }
                    }
                }
            }

            m_LoadingDone.store(true);
            cv.notify_one();
        };

        return entitiesFuture;
    }

    void EntityManager::Clear()
    {
        m_Entities.clear();
        m_LoadedEntities.clear();
    }
}