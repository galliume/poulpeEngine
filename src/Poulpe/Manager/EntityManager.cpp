#include "EntityManager.hpp"

#include "Poulpe/Component/AnimationComponent.hpp"
#include "Poulpe/Component/AnimationScript.hpp"
#include "Poulpe/Component/Renderer/RendererFactory.hpp"

#include "Poulpe/Core/TinyObjLoader.hpp"

#include <filesystem>

namespace Poulpe
{
  EntityManager::EntityManager(ComponentManager* const componentManager,
    LightManager* const lightManager,
    TextureManager* const textureManager)
    : m_ComponentManager(componentManager),
    m_LightManager(lightManager),
    m_TextureManager(textureManager)
  {
    initWorldGraph();
  }

  void EntityManager::clear()
  {
    //m_Entities.clear();
    m_HUD.clear();
    //m_LoadedEntities.clear();
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
              for (auto& [keyTex, pathTex] : data["textures"].items()) {
                textures.emplace_back(static_cast<std::string>(keyTex));
              }

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
              bool isPointLight = static_cast<bool>(data["isPointLight"]);

              std::vector<std::string> animationScripts{};
              for (auto& [keyAnim, pathAnim] : data["animationScripts"].items()) {
                animationScripts.emplace_back(static_cast<std::string>(pathAnim));
              }

              initMeshes(
                static_cast<std::string>(key),
                static_cast<std::string>(data["mesh"]),
                textures,
                static_cast<std::string>(data["shader"]),
                position,
                scale,
                rotation,
                static_cast<bool>(data["inverseTextureY"]),
                hasBbox, hasAnimation, isPointLight, animationScripts
              );
            }
          }
        } else {
          size_t count = static_cast<size_t>(data["count"]);
          form = static_cast<std::string>(data["form"]);

          for (size_t i = 0; i < count; i++) {

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
                static_cast<float>(positionData["x"]) + static_cast<float>(data["padding"]["x"]) * static_cast<float>(i),
                static_cast<float>(positionData["y"]) + static_cast<float>(data["padding"]["y"]) * static_cast<float>(i),
                static_cast<float>(positionData["z"]) + static_cast<float>(data["padding"]["z"]) * static_cast<float>(i)
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
            for (auto& [keyTex, pathTex] : data["textures"].items()) {
              textures.emplace_back(static_cast<std::string>(keyTex));
            }

            bool hasBbox = static_cast<bool>(data["hasBbox"]);
            bool hasAnimation = static_cast<bool>(data["hasAnimation"]);
            bool isPointLight = static_cast<bool>(data["isPointLight"]);

            std::vector<std::string> animationScripts{};
            for (auto& [keyAnim, pathAnim] : data["animationScripts"].items()) {
              animationScripts.emplace_back(static_cast<std::string>(pathAnim));
            }

            //@todo move init to a factory ?
            initMeshes(
              static_cast<std::string>(key),
              static_cast<std::string>(data["mesh"]),
              textures,
              static_cast<std::string>(data["shader"]),
              position,
              scale,
              rotation,
              static_cast<bool>(data["inverseTextureY"]),
              hasBbox, hasAnimation, isPointLight, animationScripts
            );

            TinyObjLoader::m_TinyObjMaterials.clear();
          }
        }
      }

      m_LoadingDone.store(true);
    };

    return entitiesFuture;
  }

  EntityNode * EntityManager::getWorldNode()
  {
    {
      std::lock_guard guard(m_MutexWorldNode);
      return m_WorldNode.get();
    }
  }

  void EntityManager::initMeshes(std::string const  & name,
    std::string const & path,
    [[maybe_unused]] std::vector<std::string> const & textureNames,
    std::string const & shader,
    glm::vec3 const & pos,
    glm::vec3 const & scale,
    glm::vec3 rotation,
    bool shouldInverseTextureY,
    bool hasBbox,
    bool hasAnimation,
    bool isPointLight,
    std::vector<std::string> animationScripts)
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

    Entity* rootMeshEntity = new Entity();
    rootMeshEntity->setName(name);
    rootMeshEntity->setVisible(false);
    EntityNode* rootMeshEntityNode = new EntityNode(rootMeshEntity);
    std::vector<Entity*> tmpToSubmit{};

    for (size_t i = 0; i < listData.size(); i++) {

      Mesh* mesh = new Mesh();
      mesh->setName(listData[i].name);
      mesh->setShaderName(shader);
      mesh->setHasAnimation(hasAnimation);
      mesh->setHasBbox(hasBbox);
      mesh->setIsPointLight(isPointLight);
      //std::vector<Mesh::BBox> bboxs{};
            
      unsigned int const tex1ID = listData[i].materialsID.at(0);

      std::string nameTexture {"_plp_empty"};
      std::string name2Texture {"_plp_empty"};
      std::string name3Texture {"_plp_empty"};
      std::string nameTextureSpecularMap;
      std::string bumpTexname;
      std::string alphaTexname;

      if (!TinyObjLoader::m_TinyObjMaterials.empty()) {
                
          //@todo material per textures...
          mesh->setMaterial(TinyObjLoader::m_TinyObjMaterials.at(listData[i].materialsID.at(0)));
            
          //@todo temp
          //@todo separate into 2 storage buffer of 3 texSample

          if (!TinyObjLoader::m_TinyObjMaterials.at(tex1ID).ambientTexname.empty()) {
              nameTexture = TinyObjLoader::m_TinyObjMaterials.at(tex1ID).ambientTexname;
          } else if (!TinyObjLoader::m_TinyObjMaterials.at(tex1ID).diffuseTexname.empty()) {
              nameTexture = TinyObjLoader::m_TinyObjMaterials.at(tex1ID).diffuseTexname;
          }

          //@todo to refacto & clean
          if (1 < listData[i].materialsID.size()) {
              unsigned int const tex2ID = listData[i].materialsID.at(1);

              if (!TinyObjLoader::m_TinyObjMaterials.at(tex2ID).ambientTexname.empty()) {
                  name2Texture = TinyObjLoader::m_TinyObjMaterials.at(tex2ID).ambientTexname;
              } else if (!TinyObjLoader::m_TinyObjMaterials.at(tex2ID).diffuseTexname.empty()) {
                  name2Texture = TinyObjLoader::m_TinyObjMaterials.at(tex2ID).diffuseTexname;
              }
          }
          if (2 < listData[i].materialsID.size()) {
            unsigned int const tex3ID = listData[i].materialsID.at(2);

            if (!TinyObjLoader::m_TinyObjMaterials.at(tex3ID).ambientTexname.empty()) {
              name3Texture = TinyObjLoader::m_TinyObjMaterials.at(tex3ID).ambientTexname;
            } else if (!TinyObjLoader::m_TinyObjMaterials.at(tex3ID).diffuseTexname.empty()) {
              name3Texture = TinyObjLoader::m_TinyObjMaterials.at(tex3ID).diffuseTexname;
            }
          }

          if (!TinyObjLoader::m_TinyObjMaterials.at(listData[i].materialId).specularTexname.empty()) {
              nameTextureSpecularMap = TinyObjLoader::m_TinyObjMaterials.at(listData[i].materialId).specularTexname;
          }

          if (!TinyObjLoader::m_TinyObjMaterials.at(listData[i].materialId).bumpTexname.empty()) {
              bumpTexname = TinyObjLoader::m_TinyObjMaterials.at(listData[i].materialId).bumpTexname;
          }

          if (!TinyObjLoader::m_TinyObjMaterials.at(listData[i].materialId).alphaTexname.empty()) {
            alphaTexname = TinyObjLoader::m_TinyObjMaterials.at(listData[i].materialId).alphaTexname;
          }
      }

      Data data{};
      data.m_Name = name + '_' + nameTexture;
      data.m_Textures.emplace_back(nameTexture);
      data.m_Textures.emplace_back(name2Texture);
      data.m_Textures.emplace_back(name3Texture);
      data.m_TextureSpecularMap = nameTextureSpecularMap;
      data.m_TextureBumpMap = bumpTexname;
      data.m_TextureAlpha = alphaTexname;
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
      ubo.model = glm::scale(ubo.model, scale);

      ubo.model = glm::translate(ubo.model, pos);

      ubo.model = glm::rotate(ubo.model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
      ubo.model = glm::rotate(ubo.model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
      ubo.model = glm::rotate(ubo.model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

      ubo.texSize = glm::vec2(0.0);

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

      /*BBox* box = new BBox();
      box->position = data.m_Ubos.at(0).model;
      box->center = center;
      box->size = size;
      box->mesh = std::make_unique<Mesh>();
      box->maxX = xMax;
      box->minX = xMin;
      box->maxY = yMax;
      box->minY = yMin;
      box->maxZ = zMax;
      box->minZ = zMin;*/

      mesh->setData(data);
      //mesh->addBBox(box);

      auto* entity = new Entity();

      if (mesh->hasAnimation()) {
        //@todo temp until lua scripting
        for (auto& anim : animationScripts) {
          auto* animationScript = new AnimationScript(anim);
          animationScript->init(m_Renderer, nullptr, nullptr);
          m_ComponentManager->addComponent<AnimationComponent>(entity->getID(), animationScript);
        }
      }

      m_ComponentManager->addComponent<MeshComponent>(entity->getID(), mesh);

      auto basicRdrImpl = RendererFactory::create<Basic>();
      m_ComponentManager->addComponent<RenderComponent>(entity->getID(), basicRdrImpl);
      basicRdrImpl->init(m_Renderer, m_TextureManager, m_LightManager);
      auto deltaTime = std::chrono::duration<float, std::milli>(0);
      basicRdrImpl->visit(deltaTime, mesh);
      
      //auto* normalEntity = new Entity();
      //auto normalRdrImpl = RendererFactory::create<Normal>();
      //normalRdrImpl->init(m_Renderer, m_TextureManager, m_LightManager);
      //m_ComponentManager->addComponent<RenderComponent>(normalEntity->getID(), normalRdrImpl);
      //normalRdrImpl->visit(deltaTime, mesh);

      auto* entityNode = new EntityNode(entity);

      rootMeshEntityNode->addChild(entityNode);

      tmpToSubmit.emplace_back(entityNode->getEntity());
      if (tmpToSubmit.size() >= 10) {
        m_Renderer->addEntities(tmpToSubmit);
        tmpToSubmit.clear();
      }
    }
    m_Renderer->addEntities(tmpToSubmit);
    tmpToSubmit.clear();

    m_WorldNode->addChild(rootMeshEntityNode);
  }

  void EntityManager::initWorldGraph()
  {
    Entity* m_World = new Entity();
    m_World->setName("_PLPWorld");
    m_World->setVisible(false);

    m_WorldNode = std::make_unique<EntityNode>(m_World);
  }
}
