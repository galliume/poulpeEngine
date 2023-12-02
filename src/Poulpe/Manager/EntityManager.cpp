#include "EntityManager.hpp"

#include "Poulpe/Renderer/Vulkan/EntityFactory.hpp"

#include <filesystem>

namespace Poulpe
{
    EntityManager::EntityManager()
    {
        initWorldGraph();
    }

    void EntityManager::initWorldGraph()
    {
      m_World = std::make_unique<Entity>();
      m_World->setName("_PLPWorld");
      m_World->setVisible(false);

      m_WorldNode = std::make_unique<EntityNode>(m_World.get());
    }

    uint32_t EntityManager::getInstancedCount()
    {
        return m_Entities.size();
    }

    void EntityManager::addEntity(std::vector<Mesh*> meshes)
    {
        for (auto& mesh : meshes) {
            uint64_t count = m_LoadedEntities.count(mesh->getName().c_str());

            if (0 != count) {
                Mesh::Data* data = mesh->getData();

                auto existingEntity = m_Entities[m_LoadedEntities[mesh->getName().c_str()][1]].get();

                existingEntity->getMesh()->addUbos(data->m_Ubos);

                UniformBufferObject ubo{};

                glm::mat4 transform = glm::translate(
                    glm::mat4(1),
                    mesh->getBBox()->center) * glm::scale(glm::mat4(1),
                        mesh->getBBox()->size);

                ubo.model = mesh->getBBox()->position * transform;
                existingEntity->getMesh()->getBBox()->mesh->addUbos({ ubo });

                m_LoadedEntities[mesh->getName()][0] += 1;
            }
            else {
                auto entity = std::make_unique<Entity>();
                //@todo change for archetype id ?
                entity->setName(mesh->getName());
                entity->setMesh(mesh);

                m_WorldNode->addChild(entity.get());

                uint32_t index = m_Entities.size();

                m_LoadedEntities.insert({ entity->getName(), { 1, index } });
                m_Entities.emplace_back(std::move(entity));
            }
        }
    }

    Entity* EntityManager::getEntityByName(std::string const & name)
    {
        auto it = std::find_if(m_Entities.cbegin(), m_Entities.cend(),
            [name](auto & entity) -> bool { return entity->getName() == name; });

        return it->get();
    }

    std::function<void()> EntityManager::load(nlohmann::json levelConfig)
    {
        m_LevelConfig = levelConfig;

        std::function<void()> entitiesFuture = [this]() {

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

                            auto positionData = data["positions"].at(0);

                            glm::vec3 position = glm::vec3(
                                static_cast<float>(positionData["x"]) * static_cast<float>(x),
                                static_cast<float>(positionData["y"]),
                                static_cast<float>(positionData["z"]) * static_cast<float>(y)
                            );

                            std::vector<std::string> textures{};
                            for (auto& [key, path]: data["textures"].items())
                                textures.emplace_back(static_cast<std::string>(key));

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
                            bool hasAnimation = static_cast<bool>(data["hasAnimation"]);

                            auto parts = initMeshes(
                                static_cast<std::string>(key),
                                static_cast<std::string>(data["mesh"]),
                                textures,
                                static_cast<std::string>(data["shader"]),
                                position,
                                scale,
                                rotation,
                                static_cast<bool>(data["inverseTextureY"])
                            );

                            for (auto & part : parts) {
                                part->setHasBbox(hasBbox);
                                part->setHasAnimation(hasAnimation);
                            }
                            addEntity(std::move(parts));
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
                        for (auto& [key, path]: data["textures"].items())
                            textures.emplace_back(static_cast<std::string>(key));

                        bool hasBbox = static_cast<bool>(data["hasBbox"]);
                        bool hasAnimation = static_cast<bool>(data["hasAnimation"]);

                        //@todo move init to a factory ?
                        auto parts = initMeshes(
                            static_cast<std::string>(key),
                            static_cast<std::string>(data["mesh"]),
                            textures,
                            static_cast<std::string>(data["shader"]),
                            position,
                            scale,
                            rotation,
                            static_cast<bool>(data["inverseTextureY"])
                        );

                        for (auto & part : parts) {
                            part->setHasBbox(hasBbox);
                            part->setHasAnimation(hasAnimation);
                        }
                        addEntity(std::move(parts));

                        TinyObjLoader::m_TinyObjMaterials.clear();
                    }
                }
            }

            m_LoadingDone.store(true);
        };

        return entitiesFuture;
    }

    void EntityManager::clear()
    {
        m_Entities.clear();
        m_HUD.clear();
        m_LoadedEntities.clear();
        m_WorldNode->clear();
        m_World.release();
    }

    std::vector<Mesh*> EntityManager::initMeshes(std::string const  & name, std::string const & path,
        std::vector<std::string> const & textureNames, std::string const & shader,
        glm::vec3 const & pos, glm::vec3 const & scale, glm::vec3 rotation,
        bool shouldInverseTextureY)
    {
        std::vector<Mesh*> meshes{};

        if (!std::filesystem::exists(path)) {
            PLP_FATAL("mesh file {} does not exits.", path);
            throw std::runtime_error("error loading a mesh file.");
        }

        //if (m_ObjLoaded.contains(path)) return meshes;
       
        //m_ObjLoaded.insert(path);

        //@todo not reload an already loaded obj
        std::vector<TinyObjData> listData = TinyObjLoader::loadData(path, shouldInverseTextureY);


        for (size_t i = 0; i < listData.size(); i++) {

            Mesh* mesh = new Mesh();
            mesh->setName(name + '_' + std::to_string(i));
            mesh->setShaderName(shader);

            std::vector<Mesh::BBox> bboxs{};
            
            auto nameTexture = textureNames[listData[i].materialId];

            if (!TinyObjLoader::m_TinyObjMaterials.empty()) {
                
                mesh->setMaterial(TinyObjLoader::m_TinyObjMaterials.at(listData[i].materialId));
            
                //@todo temp
                if (!TinyObjLoader::m_TinyObjMaterials.at(listData[i].materialId).diffuseTexname.empty()) {
                    nameTexture = TinyObjLoader::m_TinyObjMaterials.at(listData[i].materialId).diffuseTexname;
                }
            }

            Mesh::Data data{};
            data.m_Name = name + '_' + nameTexture;
            data.m_Texture = nameTexture;
            data.m_Vertices = listData[i].vertices;
            data.m_Indices = listData[i].indices;
            data.m_OriginPos = pos;
            data.m_CurrentPos = pos;
            data.m_OriginScale = scale;
            data.m_CurrentScale = scale;
            data.m_OriginRotation = rotation;
            data.m_CurrentRotation = rotation;

            UniformBufferObject ubo{};
            ubo.model = glm::mat4(1.0f);
            ubo.model = glm::translate(ubo.model, pos);
            ubo.model = glm::scale(ubo.model, scale);

            ubo.model = glm::rotate(ubo.model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            ubo.model = glm::rotate(ubo.model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            ubo.model = glm::rotate(ubo.model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

            //ubo.view = glm::mat4(1.0f);
            data.m_Ubos.emplace_back(ubo);

            float xMax = data.m_Vertices.at(0).pos.x;
            float yMax = data.m_Vertices.at(0).pos.y;
            float zMax = data.m_Vertices.at(0).pos.z;

            float xMin = xMax;
            float yMin = yMax;
            float zMin = zMax;

            for (size_t j = 0; j < data.m_Vertices.size(); j++) {

                glm::vec3 vertex = glm::vec4(data.m_Vertices.at(j).pos, 1.0f);

                float x = vertex.x;
                float y = vertex.y;
                float z = vertex.z;

                if (x > xMax) xMax = x;
                if (x < xMin) xMin = x;
                if (y < yMin) yMin = y;
                if (y > yMax) yMax = y;
                if (z > zMax) zMax = z;
                if (z < zMin) zMin = z;
            }

            glm::vec3 center = glm::vec3((xMin + xMax) / 2, (yMin + yMax) / 2, (zMin + zMax) / 2);
            glm::vec3 size = glm::vec3((xMax - xMin) / 2, (yMax - yMin) / 2, (zMax - zMin) / 2);

            Mesh::BBox* box = new Mesh::BBox();
            box->position = data.m_Ubos.at(0).model;
            box->center = center;
            box->size = size;
            box->mesh = std::make_unique<Mesh>();
            box->maxX = xMax;
            box->minX = xMin;
            box->maxY = yMax;
            box->minY = yMin;
            box->maxZ = zMax;
            box->minZ = zMin;

            mesh->setData(data);
            mesh->addBBox(box);

            meshes.emplace_back(std::move(mesh));
        }

        return meshes;
    }
}