#include "EntityManager.hpp"

#include "Poulpe/Component/AnimationComponent.hpp"
#include "Poulpe/Component/AnimationScript.hpp"
#include "Poulpe/Component/BoneAnimationComponent.hpp"
#include "Poulpe/Component/BoneAnimationScript.hpp"
#include "Poulpe/Component/Renderer/RendererFactory.hpp"

#include "Poulpe/Core/AssimpLoader.hpp"

#include "Poulpe/Manager/ComponentManager.hpp"

#include "glm/glm.hpp"

#include <filesystem>
#include <future>

namespace Poulpe
{
  EntityManager::EntityManager(ComponentManager* const component_manager,
    LightManager* const light_manager,
    TextureManager* const texture_manager)
    : _component_manager(component_manager),
    _light_manager(light_manager),
    _texture_manager(texture_manager)
  {
    initWorldGraph();
  }

  void EntityManager::clear()
  {
    //_Entities.clear();
    _HUD.clear();
    //_LoadedEntities.clear();
  }

  std::function<void()> EntityManager::load(nlohmann::json const& levelConfig)
  {
    _LevelConfig = levelConfig;

    return [this]() {
      std::ranges::for_each(_LevelConfig["entities"].items(), [&](auto const& entityConf) {

        auto const& key = entityConf.key();
        auto const& data = entityConf.value();
        size_t const count = data["count"].template get<size_t>();

        for (size_t i{ 0 }; i < count; i++) {
          std::jthread entity([&]() {
            initMeshes(key, data);
          });
          entity.detach();
        }
      });

      _LoadingDone.store(true);
    };
  }

  EntityNode * EntityManager::getWorldNode()
  {
    {
      std::lock_guard guard(_MutexWorldNode);
      return _WorldNode.get();
    }
  }

  void EntityManager::initMeshes(std::string const& name, nlohmann::json const data)
  {
    //std::vector<Mesh*> meshes{};
    //if (_ObjLoaded.contains(path)) return meshes;

    //_ObjLoaded.insert(path);

    //@todo not reload an already loaded obj
    Entity* rootMeshEntity = new Entity();
    rootMeshEntity->setName(name);
    rootMeshEntity->setVisible(false);

    auto const& path = data["mesh"].template get<std::string>();
    auto const inverse_texture_y = data["inverseTextureY"].template get<bool>();

    auto callback = [this, data, path, rootMeshEntity](
      PlpMeshData const _data,
      std::vector<material_t> const materials,
      bool const exists,
      std::vector<Animation> const animations,
      std::vector<Position> const positions,
      std::vector<Rotation> const rotations,
      std::vector<Scale> const scales) {

    auto const& positionData = data["positions"].at(0);

    glm::vec3 position{};
    position = glm::vec3(
      positionData["x"].template get<float>(),
      positionData["y"].template get<float>(),
      positionData["z"].template get<float>()
    );

    auto const& scaleData = data["scales"].at(0);
    auto const& rotationData = data["rotations"].at(0);

    glm::vec3 const scale = glm::vec3(
      scaleData["x"].template get<float>(),
      scaleData["y"].template get<float>(),
      scaleData["z"].template get<float>()
    );
    glm::vec3 const rotation = glm::vec3(
      rotationData["x"].template get<float>(),
      rotationData["y"].template get<float>(),
      rotationData["z"].template get<float>()
    );

    std::vector<std::string> textures{};
    textures.reserve(data["textures"].size());
    for (auto& [keyTex, pathTex] : data["textures"].items()) {
      textures.emplace_back(static_cast<std::string>(keyTex));
    }

    bool const hasBbox = data["hasBbox"].template get<bool>();
    bool const hasAnimation = data["hasAnimation"].template get<bool>();
    bool const isPointLight = data["isPointLight"].template get<bool>();

    std::vector<std::string> animationScripts{};
    animationScripts.reserve(data["animationScripts"].size());
    for (auto& [keyAnim, pathAnim] : data["animationScripts"].items()) {
      animationScripts.emplace_back(static_cast<std::string>(pathAnim));
    }

    auto shader = data["shader"].template get<std::string>();

    EntityOptions entityOptions = {
      shader, position, scale, rotation,
      data["hasBbox"].template get<bool>(),
      data["hasAnimation"].template get<bool>(),
      data["isPointLight"].template get<bool>(),
      animationScripts,
      data["hasShadow"].template get<bool>(),
      data["inverseTextureY"].template get<bool>(),
      data["isIndexed"].template get<bool>()
    };

      EntityNode* rootMeshEntityNode = new EntityNode(rootMeshEntity);

      std::unique_ptr<Mesh>mesh = std::make_unique<Mesh>();
      mesh->setName(_data.name);
      mesh->setShaderName(entityOptions.shader);
      mesh->setHasAnimation(entityOptions.hasAnimation);
      mesh->setHasBbox(entityOptions.hasBbox);
      mesh->setIsPointLight(entityOptions.isPointLight);
      mesh->setHasShadow(entityOptions.hasShadow);
      mesh->setIsIndexed(entityOptions.isIndexed);
      //std::vector<Mesh::BBox> bboxs{};

      unsigned int const tex1ID = _data.materialsID.at(0);

      std::string nameTexture{ "_plp_empty" };
      std::string name2Texture{ "_plp_empty" };
      std::string name3Texture{ "_plp_empty" };
      std::string nameTextureSpecularMap{ "_plp_empty" };
      std::string bumpTexname{ "_plp_empty" };
      std::string alphaTexname{ "_plp_empty" };

      if (!materials.empty()) {

        //@todo material per textures...
        mesh->setMaterial(materials.at(_data.materialsID.at(0)));

        //@todo temp
        //@todo separate into 2 storage buffer of 3 texSample
        auto const& tex1 = materials.at(tex1ID);

        if (!tex1.ambientTexname.empty()) {
          nameTexture = tex1.ambientTexname;
        } else if (!tex1.diffuseTexname.empty()) {
          nameTexture = tex1.diffuseTexname;
        }

        //@todo to refacto & clean
        if (1 < _data.materialsID.size()) {
          auto const& tex2 = materials.at(_data.materialsID.at(1));

          if (!tex2.ambientTexname.empty()) {
            name2Texture = tex2.ambientTexname;
          } else if (!tex2.diffuseTexname.empty()) {
            name2Texture = tex2.diffuseTexname;
          }
        }
        if (2 < _data.materialsID.size()) {
          auto const& tex3 = materials.at(_data.materialsID.at(2));

          if (!tex3.ambientTexname.empty()) {
            name3Texture = tex3.ambientTexname;
          }
          else if (!tex3.diffuseTexname.empty()) {
            name3Texture = tex3.diffuseTexname;
          }
        }

        auto const& mat = materials.at(_data.materialId);

        if (!mat.specularTexname.empty()) {
          nameTextureSpecularMap = mat.specularTexname;
        }

        if (!mat.bumpTexname.empty()) {
          bumpTexname = mat.bumpTexname;
        }

        if (!mat.alphaTexname.empty()) {
          alphaTexname = mat.alphaTexname;
        }
      }

      Data data{};
      data._name = _data.name + '_' + nameTexture;
      data._textures.emplace_back(nameTexture);
      data._textures.emplace_back(name2Texture);
      data._textures.emplace_back(name3Texture);
      data._specular_map = nameTextureSpecularMap;
      data._bump_map = bumpTexname;
      data._alpha = alphaTexname;
      data._vertices = _data.vertices;
      data._Indices = _data.indices;
      data._origin_pos = entityOptions.pos;
      data._current_pos = entityOptions.pos;
      data._origin_scale = entityOptions.scale;
      data._current_scale = entityOptions.scale;
      data._origin_rotation = entityOptions.rotation;
      data._current_rotation = entityOptions.rotation;

      UniformBufferObject ubo{};
      ubo.model = glm::mat4(1.0f);
      ubo.model = glm::scale(ubo.model, entityOptions.scale);

      ubo.model = glm::translate(ubo.model, entityOptions.pos);

      ubo.model = glm::rotate(ubo.model, glm::radians(entityOptions.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
      ubo.model = glm::rotate(ubo.model, glm::radians(entityOptions.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
      ubo.model = glm::rotate(ubo.model, glm::radians(entityOptions.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

      ubo.texSize = glm::vec2(0.0);

      //ubo.view = glm::mat4(1.0f);
      data._ubos.emplace_back(ubo);

      glm::vec3 center = glm::vec3(0.0);
      glm::vec3 size = glm::vec3(0.0);

      //if (data._vertices.size() > 0) {
      //  float xMax = data._vertices.at(0).pos.x;
      //  float yMax = data._vertices.at(0).pos.y;
      //  float zMax = data._vertices.at(0).pos.z;

      //  float xMin = xMax;
      //  float yMin = yMax;
      //  float zMin = zMax;

      //  for (size_t j = 0; j < data._vertices.size(); j++) {

      //    glm::vec3 vertex = glm::vec4(data._vertices.at(j).pos, 1.0f);

      //    float x = vertex.x;
      //    float y = vertex.y;
      //    float z = vertex.z;

      //    if (x > xMax) xMax = x;
      //    if (x < xMin) xMin = x;
      //    if (y < yMin) yMin = y;
      //    if (y > yMax) yMax = y;
      //    if (z > zMax) zMax = z;
      //    if (z < zMin) zMin = z;
      //  }

      //  center = glm::vec3((xMin + xMax) / 2, (yMin + yMax) / 2, (zMin + zMax) / 2);
      //  size = glm::vec3((xMax - xMin) / 2, (yMax - yMin) / 2, (zMax - zMin) / 2);
      //}

      /*BBox* box = new BBox();
      box->position = data._ubos.at(0).model;
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
      entity->setName(_data.name);

      if (mesh->hasAnimation()) {
        //@todo temp until lua scripting
        for (auto& anim : entityOptions.animationScripts) {
          auto animationScript = std::make_unique<AnimationScript>(anim);
          animationScript->init(_renderer, nullptr, nullptr);
          _component_manager->add<AnimationComponent>(entity->getID(), std::move(animationScript));
        }

        if (!animations.empty()) {
          auto boneAnimationScript = std::make_unique<BoneAnimationScript>(animations, positions, rotations, scales);
          _component_manager->add<BoneAnimationComponent>(
            entity->getID(), std::move(boneAnimationScript));
        }
      }
      auto basicRdrImpl = RendererFactory::create<Basic>();

      basicRdrImpl->init(_renderer, _texture_manager, _light_manager);
      auto const deltaTime = std::chrono::duration<float, std::milli>(0);
      (*basicRdrImpl)(deltaTime, mesh.get());

      _component_manager->add<RenderComponent>(entity->getID(), std::move(basicRdrImpl));
      _component_manager->add<MeshComponent>(entity->getID(), std::move(mesh));
      {
        std::shared_lock guard(_SharedMutex);
        auto* entityNode = rootMeshEntityNode->addChild(new EntityNode(entity));
        _WorldNode->addChild(rootMeshEntityNode);
        _renderer->addEntity(entityNode->getEntity());
      }
    };
    AssimpLoader::loadData(path, inverse_texture_y, callback);
  }

  void EntityManager::initWorldGraph()
  {
    Entity* _World = new Entity();
    _World->setName("_PLPWorld");
    _World->setVisible(false);

    _WorldNode = std::make_unique<EntityNode>(_World);
  }
}
