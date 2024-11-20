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
  EntityManager::EntityManager(
    ComponentManager* const component_manager,
    LightManager* const light_manager,
    TextureManager* const texture_manager)
    : _component_manager(component_manager)
    , _light_manager(light_manager)
    , _texture_manager(texture_manager)
  {
    initWorldGraph();
  }

  void EntityManager::clear()
  {
    //_Entities.clear();
    _HUD.clear();
    //_LoadedEntities.clear();
  }

  std::function<void()> EntityManager::load(nlohmann::json const& lvl_config)
  {
    _lvl_config = lvl_config;

    return [this]() {
      std::ranges::for_each(_lvl_config["entities"].items(), [&](auto const& conf) {

        auto const& key = conf.key();
        auto const& data = conf.value();
        size_t const count = data["count"].template get<size_t>();

        for (size_t i{ 0 }; i < count; i++) {
          std::jthread entity([&]() {
            initMeshes(key, data);
          });
          entity.detach();
        }
      });
    };
  }

  EntityNode * EntityManager::getWorldNode()
  {
      return _world_node.get();
  }

  void EntityManager::initMeshes(std::string const& name, nlohmann::json const& data)
  {
    //std::vector<Mesh*> meshes{};
    //if (_ObjLoaded.contains(path)) return meshes;

    //_ObjLoaded.insert(path);

    //@todo not reload an already loaded obj
    Entity* root_mesh_entity = new Entity();
    root_mesh_entity->setName(name);
    root_mesh_entity->setVisible(false);

    auto const& path = data["mesh"].template get<std::string>();
    auto const inverse_texture_y = data["inverseTextureY"].template get<bool>();

    auto callback = [this, data, path, root_mesh_entity](
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

    auto const& scale_data = data["scales"].at(0);
    auto const& rotation_data = data["rotations"].at(0);

    glm::vec3 const scale = glm::vec3(
      scale_data["x"].template get<float>(),
      scale_data["y"].template get<float>(),
      scale_data["z"].template get<float>()
    );
    glm::vec3 const rotation = glm::vec3(
      rotation_data["x"].template get<float>(),
      rotation_data["y"].template get<float>(),
      rotation_data["z"].template get<float>()
    );

    std::vector<std::string> textures{};
    textures.reserve(data["textures"].size());
    for (auto& [keyTex, pathTex] : data["textures"].items()) {
      textures.emplace_back(static_cast<std::string>(keyTex));
    }

    bool const has_bbox = data["hasBbox"].template get<bool>();
    bool const has_animation = data["hasAnimation"].template get<bool>();
    bool const is_point_light = data["isPointLight"].template get<bool>();

    std::vector<std::string> animation_scripts{};
    animation_scripts.reserve(data["animationScripts"].size());
    for (auto& [key_anim, path_anim] : data["animationScripts"].items()) {
      animation_scripts.emplace_back(static_cast<std::string>(path_anim));
    }

    auto shader = data["shader"].template get<std::string>();

    EntityOptions entity_opts = {
      shader, position, scale, rotation,
      data["hasBbox"].template get<bool>(),
      data["hasAnimation"].template get<bool>(),
      data["isPointLight"].template get<bool>(),
      animation_scripts,
      data["hasShadow"].template get<bool>(),
      data["inverseTextureY"].template get<bool>(),
      data["isIndexed"].template get<bool>()
    };

      EntityNode* root_mesh_entity_node = new EntityNode(root_mesh_entity);

      std::unique_ptr<Mesh>mesh = std::make_unique<Mesh>();
      mesh->setName(_data.name);
      mesh->setShaderName(entity_opts.shader);
      mesh->setHasAnimation(entity_opts.has_animation);
      mesh->setIsPointLight(entity_opts.is_point_light);
      mesh->setHasShadow(entity_opts.has_shadow);
      mesh->setIsIndexed(entity_opts.is_indexed);
      //std::vector<Mesh::BBox> bboxs{};

      unsigned int const tex1ID = _data.materials_ID.at(0);

      std::string name_texture{ "_plp_empty" };
      std::string name_specular_map{ "_plp_empty" };
      std::string name_bump_map{ "_plp_empty" };
      std::string name_alpha_map{ "_plp_empty" };

      if (!materials.empty()) {

        //@todo material per textures...
        mesh->setMaterial(materials.at(_data.materials_ID.at(0)));

        //@todo temp
        //@todo separate into 2 storage buffer of 3 texSample
        auto const& tex1 = materials.at(tex1ID);

        if (!tex1.name_texture_ambient.empty()) {
          name_texture = tex1.name_texture_ambient;
        } else if (!tex1.name_texture_diffuse.empty()) {
          name_texture = tex1.name_texture_diffuse;
        }

        auto const& mat = materials.at(_data.material_ID);

        if (!mat.name_texture_specular.empty()) {
          name_specular_map = mat.name_texture_specular;
        }

        if (!mat.name_texture_bump.empty()) {
          name_bump_map = mat.name_texture_bump;
        }

        if (!mat.name_texture_alpha.empty()) {
          name_alpha_map = mat.name_texture_alpha;
        }
      }

      Data data{};
      data._name = _data.name + '_' + name_texture;
      data._textures.emplace_back(name_texture);
      data._specular_map = name_specular_map;
      data._bump_map = name_bump_map;
      data._alpha = name_alpha_map;
      data._vertices = _data.vertices;
      data._indices = _data.indices;
      data._origin_pos = entity_opts.pos;
      data._current_pos = entity_opts.pos;
      data._origin_scale = entity_opts.scale;
      data._current_scale = entity_opts.scale;
      data._origin_rotation = entity_opts.rotation;
      data._current_rotation = entity_opts.rotation;

      UniformBufferObject ubo{};
      ubo.model = glm::mat4(1.0f);
      ubo.model = glm::scale(ubo.model, entity_opts.scale);

      ubo.model = glm::translate(ubo.model, entity_opts.pos);

      ubo.model = glm::rotate(ubo.model, glm::radians(entity_opts.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
      ubo.model = glm::rotate(ubo.model, glm::radians(entity_opts.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
      ubo.model = glm::rotate(ubo.model, glm::radians(entity_opts.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

      ubo.tex_size = glm::vec2(0.0);

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

      if (mesh->has_animation()) {
        //@todo temp until lua scripting
        for (auto& anim : entity_opts.animation_scripts) {
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
        std::shared_lock guard(_mutex_shared);
        auto* entityNode = root_mesh_entity_node->addChild(new EntityNode(entity));
        _world_node->addChild(root_mesh_entity_node);
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

    _world_node = std::make_unique<EntityNode>(_World);
  }
}
