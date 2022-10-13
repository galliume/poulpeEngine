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

    std::vector<std::future<void>> EntityManager::Load()
    {
        std::vector<std::future<void>> futures{};

        std::future worldFuture = std::async(std::launch::async, [this]() {

            std::shared_ptr<Mesh> entity = std::make_shared<Mesh>();

            entity->Init(
                "doggo",
                "assets/mesh/dog/dog.obj",
                { "dog" },
                "main",
                glm::vec3(0.8f, 0.4f, -1.2f),
                glm::vec3(0.12f, 0.12f, 0.12f));

            AddEntity(entity);

            float fencePos = -2.4;
            for (int x = 0; x < 6; x++) {
                std::shared_ptr<Mesh> entity = std::make_shared<Mesh>();
                entity->Init(
                    "fence",
                    "assets/mesh/fence/fence.obj",
                    { "fence" },
                    "main",
                    glm::vec3(4.0f, 0.22f, fencePos),
                    glm::vec3(0.005f, 0.005f, 0.005f),
                    glm::vec3(0.0f, 1.0f, 0.0f),
                    -90.0f);

                AddEntity(entity);
                fencePos += 0.85f;
            }

            fencePos = 3.55f;
            for (int x = 0; x < 6; x++) {
                std::shared_ptr<Mesh> entity = std::make_shared<Mesh>();
                entity->Init(
                    "fence",
                    "assets/mesh/fence/fence.obj",
                    { "fence" },
                    "main",
                    glm::vec3(fencePos, 0.22f, 2.3f),
                    glm::vec3(0.005f, 0.005f, 0.005f),
                    glm::vec3(0.0f, 1.0f, 0.0f),
                    -180.0f);

                AddEntity(entity);
                fencePos -= 0.85f;
            }

            for (int x = -7; x < 7; x++) {
                for (int y = -5; y < 15; y++) {
                    std::shared_ptr<Rbk::Mesh> entity = std::make_shared<Rbk::Mesh>();
                    entity->Init(
                        "ground_cube",
                        "assets/mesh/minecraft/Grass_Block.obj",
                        { "minecraft_grass" },
                        "main",
                        glm::vec3(-0.20f * static_cast<float>(x), 0.0f, -0.20f * static_cast<float>(y)),
                        glm::vec3(0.1f, 0.1f, 0.1f));

                    AddEntity(entity);
                }
            }

            for (int x = -7; x < 7; x++) {
                for (int y = -15; y < -5; y++) {
                    std::shared_ptr<Rbk::Mesh> entity = std::make_shared<Rbk::Mesh>();
                    entity->Init(
                        "ground_cube1",
                        "assets/mesh/minecraft/Grass_Block.obj",
                        { "minecraft_grass" },
                        "main",
                        glm::vec3(-0.20f * static_cast<float>(x), 0.0f, -0.20f * static_cast<float>(y)),
                        glm::vec3(0.1f, 0.1f, 0.1f));

                    AddEntity(entity);
                }
            }
            for (int x = -22; x < -7; x++) {
                for (int y = -5; y < 15; y++) {
                    std::shared_ptr<Rbk::Mesh> entity = std::make_shared<Rbk::Mesh>();
                    entity->Init(
                        "ground_cube_2",
                        "assets/mesh/minecraft/Grass_Block.obj",
                        { "minecraft_grass" },
                        "main",
                        glm::vec3(-0.20f * static_cast<float>(x), 0.0f, -0.20f * static_cast<float>(y)),
                        glm::vec3(0.1f, 0.1f, 0.1f));

                    AddEntity(entity);
                }
            }
            for (int x = -22; x < -7; x++) {
                for (int y = -15; y < -5; y++) {
                    std::shared_ptr<Rbk::Mesh> entity = std::make_shared<Rbk::Mesh>();
                    entity->Init(
                        "ground_cube3",
                        "assets/mesh/minecraft/Grass_Block.obj",
                        { "minecraft_grass" },
                        "main",
                        glm::vec3(-0.20f * static_cast<float>(x), 0.0f, -0.20f * static_cast<float>(y)),
                        glm::vec3(0.1f, 0.1f, 0.1f));

                    AddEntity(entity);
                }
            }
            for (int x = 7; x < 22; x++) {
                for (int y = -15; y < -5; y++) {
                    std::shared_ptr<Rbk::Mesh> entity = std::make_shared<Rbk::Mesh>();
                    entity->Init(
                        "ground_cube4",
                        "assets/mesh/minecraft/Grass_Block.obj",
                        { "minecraft_grass" },
                        "main",
                        glm::vec3(-0.20f * static_cast<float>(x), 0.0f, -0.20f * static_cast<float>(y)),
                        glm::vec3(0.1f, 0.1f, 0.1f));

                    AddEntity(entity);
                }
            }
            });

        std::future worldFutureTer = std::async(std::launch::async, [this]() {

            for (int x = 7; x < 22; x++) {
                for (int y = -5; y < 15; y++) {
                    std::shared_ptr<Rbk::Mesh> entity = std::make_shared<Rbk::Mesh>();
                    entity->Init(
                        "water_cube_1",
                        "assets/mesh/minecraft/Grass_Block.obj",
                        { "minecraft_water" },
                        "main",
                        glm::vec3(-0.20f * static_cast<float>(x), -0.1f, -0.20f * static_cast<float>(y)),
                        glm::vec3(0.1f, 0.1f, 0.1f));

                    AddEntity(entity);
                }
            }

            for (int x = -22; x < 22; x++) {
                for (int y = 15; y < 22; y++) {
                    std::shared_ptr<Rbk::Mesh> entity = std::make_shared<Rbk::Mesh>();
                    entity->Init(
                        "water_cube_2",
                        "assets/mesh/minecraft/Grass_Block.obj",
                        { "minecraft_water" },
                        "main",
                        glm::vec3(-0.20f * static_cast<float>(x), -0.1f, -0.20f * static_cast<float>(y)),
                        glm::vec3(0.1f, 0.1f, 0.1f));

                    AddEntity(entity);
                }
            }

            for (int x = -22; x < 22; x++) {
                for (int y = 22; y < 29; y++) {
                    std::shared_ptr<Rbk::Mesh> entity = std::make_shared<Rbk::Mesh>();
                    entity->Init(
                        "water_cube_3",
                        "assets/mesh/minecraft/Grass_Block.obj",
                        { "minecraft_water" },
                        "main",
                        glm::vec3(-0.20f * static_cast<float>(x), -0.1f, -0.20f * static_cast<float>(y)),
                        glm::vec3(0.1f, 0.1f, 0.1f));

                    AddEntity(entity);
                }
            }

            std::shared_ptr<Rbk::Mesh> entity = std::make_shared<Rbk::Mesh>();
            entity->Init(
                "campfire",
                "assets/mesh/campfire/Campfire.obj",
                { "campfire_tex" },
                "main",
                glm::vec3(0.8f, 0.2f, -0.4f),
                glm::vec3(0.002f, 0.002f, 0.002f));

            AddEntity(entity);

            std::shared_ptr<Rbk::Mesh> entity1 = std::make_shared<Rbk::Mesh>();
            entity1->Init(
                "dead_tree",
                "assets/mesh/tree/dead_tree.obj",
                { "trunk_tree_cartoon" },
                "main",
                glm::vec3(-1.0f, 0.19f, -1.4f),
                glm::vec3(0.01f, 0.01f, 0.01f));

            AddEntity(entity1);

            std::shared_ptr<Rbk::Mesh> entity2 = std::make_shared<Rbk::Mesh>();
            entity2->Init(
                "dead_tree",
                "assets/mesh/tree/dead_tree.obj",
                { "trunk_tree_cartoon" },
                "main",
                glm::vec3(1.0f, 0.19f, 1.0f),
                glm::vec3(0.01f, 0.01f, 0.01f),
                glm::vec3(0.0f, 1.0f, 0.0f),
                -20.0f);

            AddEntity(entity2);

            std::shared_ptr<Rbk::Mesh> entity3 = std::make_shared<Rbk::Mesh>();
            entity3->Init(
                "dead_tree",
                "assets/mesh/tree/dead_tree.obj",
                { "trunk_tree_cartoon" },
                "main",
                glm::vec3(1.0f, 0.18f, -2.0f),
                glm::vec3(0.01f, 0.01f, 0.01f));

            AddEntity(entity3);

            std::shared_ptr<Rbk::Mesh> entity4 = std::make_shared<Rbk::Mesh>();
            entity4->Init(
                "dead_tree",
                "assets/mesh/tree/dead_tree.obj",
                { "trunk_tree_cartoon" },
                "main",
                glm::vec3(3.0f, 0.18f, -2.2f),
                glm::vec3(0.01f, 0.01f, 0.01f));

            AddEntity(entity);
            });

        std::future worldBisFuture = std::async(std::launch::async, [this]() {
            std::shared_ptr<Rbk::Mesh> entity5 = std::make_shared<Rbk::Mesh>();
            entity5->Init(
                "dead_tree",
                "assets/mesh/tree/dead_tree.obj",
                { "trunk_tree_cartoon" },
                "main",
                glm::vec3(3.0f, 0.18f, -0.6f),
                glm::vec3(0.01f, 0.01f, 0.01f));

            AddEntity(entity5);

            std::shared_ptr<Rbk::Mesh> entity6 = std::make_shared<Rbk::Mesh>();
            entity6->Init(
                "tree",
                "assets/mesh/tree/tree.obj",
                { "tree_top_tex", "trunk_tree_cartoon" },
                "main",
                glm::vec3(3.0f, 0.18f, -0.6f),
                glm::vec3(0.0008f, 0.0008f, 0.0008f));

            AddEntity(entity6);

            std::shared_ptr<Rbk::Mesh> entity7 = std::make_shared<Rbk::Mesh>();
            entity7->Init(
                "tree",
                "assets/mesh/tree/tree.obj",
                { "tree_top_tex", "trunk_tree_cartoon" },
                "main",
                glm::vec3(-0.25f, 0.19f, -1.2f),
                glm::vec3(0.0008f, 0.0008f, 0.0008f));

            AddEntity(entity7);

            std::shared_ptr<Rbk::Mesh> entity8 = std::make_shared<Rbk::Mesh>();
            entity8->Init(
                "tree",
                "assets/mesh/tree/tree.obj",
                { "tree_top_tex", "trunk_tree_cartoon" },
                "main",
                glm::vec3(0.7f, 0.19f, -1.6f),
                glm::vec3(0.0008f, 0.0008f, 0.0008f));

            AddEntity(entity8);

            std::shared_ptr<Rbk::Mesh> entity9 = std::make_shared<Rbk::Mesh>();
            entity9->Init(
                "tree",
                "assets/mesh/tree/tree.obj",
                { "tree_top_tex", "trunk_tree_cartoon" },
                "main",
                glm::vec3(1.2f, 0.19f, -0.9f),
                glm::vec3(0.0008f, 0.0008f, 0.0008f));

            AddEntity(entity9);

            std::shared_ptr<Rbk::Mesh> entity10 = std::make_shared<Rbk::Mesh>();
            entity10->Init(
                "tree",
                "assets/mesh/tree/tree.obj",
                { "tree_top_tex", "trunk_tree_cartoon" },
                "main",
                glm::vec3(2.2f, 0.19f, -1.9f),
                glm::vec3(0.0008f, 0.0008f, 0.0008f));

            AddEntity(entity10);

            std::shared_ptr<Rbk::Mesh> entity11 = std::make_shared<Rbk::Mesh>();
            entity11->Init(
                "tree",
                "assets/mesh/tree/tree.obj",
                { "tree_top_tex", "trunk_tree_cartoon" },
                "main",
                glm::vec3(3.2f, 0.19f, -0.2f),
                glm::vec3(0.0008f, 0.0008f, 0.0008f));

            AddEntity(entity11);

            std::shared_ptr<Rbk::Mesh> entity12 = std::make_shared<Rbk::Mesh>();
            entity12->Init(
                "tree",
                "assets/mesh/tree/tree.obj",
                { "tree_top_tex", "trunk_tree_cartoon" },
                "main",
                glm::vec3(2.8f, 0.19f, 0.5f),
                glm::vec3(0.0008f, 0.0008f, 0.0008f));

            AddEntity(entity12);

            std::shared_ptr<Rbk::Mesh> entity13 = std::make_shared<Rbk::Mesh>();
            entity13->Init(
                "tree",
                "assets/mesh/tree/tree.obj",
                { "tree_top_tex", "trunk_tree_cartoon" },
                "main",
                glm::vec3(2.0f, 0.19f, 0.5f),
                glm::vec3(0.0008f, 0.0008f, 0.0008f));

            AddEntity(entity13);

            std::shared_ptr<Rbk::Mesh> entity14 = std::make_shared<Rbk::Mesh>();
            entity14->Init(
                "moon",
                "assets/mesh/moon/moon.obj",
                { "moon" },
                "ambient_light",
                glm::vec3(0.5f, 4.5f, -3.00f),
                glm::vec3(0.2f, 0.2f, 0.2f),
                glm::vec3(1.0f),
                0.0f,
                false);

            AddEntity(entity14);

            std::shared_ptr<Rbk::Mesh> entity15 = std::make_shared<Rbk::Mesh>();
            entity15->Init(
                "house",
                "assets/mesh/house/midpoly_town_house_01.obj",
                { "dark_wood", "rocks", "rooftiles", "bright_wood" },
                "main",
                glm::vec3(-0.5f, 0.19f, -0.20f),
                glm::vec3(0.1f, 0.1f, 0.1f),
                glm::vec3(0.0f, 1.0f, 0.0f),
                -80.0f,
                false);

            AddEntity(entity15);

            std::shared_ptr<Rbk::Mesh> entity16 = std::make_shared<Rbk::Mesh>();
            entity16->Init(
                "dog_house",
                "assets/mesh/doghouse/doghouse0908.obj",
                { "dog_base_color" },
                "main",
                glm::vec3(0.7f, 0.20f, -2.0f),
                glm::vec3(0.3f, 0.3f, 0.3f),
                glm::vec3(0.0f, 1.0f, 0.0f),
                -30.0f);

            AddEntity(entity16);
        });

        futures.emplace_back(std::move(worldFuture));
        futures.emplace_back(std::move(worldBisFuture));
        futures.emplace_back(std::move(worldFutureTer));

        return futures;
    }
}