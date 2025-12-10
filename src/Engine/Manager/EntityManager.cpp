module;


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/fwd.hpp>

#include <nlohmann/json.hpp>

module Engine.Managers.EntityManager;

import std;

import Engine.Animation.AnimationScript;
import Engine.Animation.BoneAnimationScript;
import Engine.Component.Components;
import Engine.Component.Entity;
import Engine.Component.EntityNode;
import Engine.Core.AssimpLoader;
import Engine.Core.MeshTypes;
import Engine.Core.PlpTypedef;
import Engine.Managers.ComponentManager;
import Engine.Renderer;
import Engine.Renderer.Renderers;
import Engine.Renderer.Mesh;
import Engine.Renderer.RendererComponent;
import Engine.Renderer.RendererComponentFactory;
import Engine.Renderer.RendererComponentTypes;

namespace Poulpe
{
  EntityManager::EntityManager(
    ComponentManager* const component_manager,
    LightManager* const light_manager,
    TextureManager* const texture_manager,
    Buffer& light_buffer)
    : _component_manager(component_manager)
    , _light_manager(light_manager)
    , _texture_manager(texture_manager)
    , _light_buffer(light_buffer)
  {
    initWorldGraph();
  }

  void EntityManager::clear()
  {
    _world_node->clear();
  }

  std::function<void()> EntityManager::load(nlohmann::json const& lvl_config)
  {
    _lvl_config = lvl_config;

    return [this]() {
      std::ranges::for_each(_lvl_config["entities"].items(), [&](auto const& conf) {

        auto const& key = conf.key();
        auto const& data = conf.value();

        initMeshes(key, data);
      });
    };
  }

  EntityNode * EntityManager::getWorldNode()
  {
    return _world_node.get();
  }

  void EntityManager::initMeshes(std::string const& name, nlohmann::json const& raw_data)
  {
    //std::vector<Mesh*> meshes{};
    //if (_ObjLoaded.contains(path)) return meshes;

    //_ObjLoaded.insert(path);

    //@todo not reload an already loaded obj
    Entity* root_mesh_entity = new Entity();
    root_mesh_entity->setName(name);
    root_mesh_entity->setVisible(false);

    EntityNode* root_mesh_entity_node = new EntityNode(root_mesh_entity);

    auto const& path = raw_data["mesh"].template get<std::string>();
    auto const flip_Y = raw_data["flipY"].template get<bool>();

    auto callback = [this, raw_data, path, root_mesh_entity_node](
      PlpMeshData const _data,
      std::vector<material_t> const materials,
      bool const,
      std::vector<Animation> const animations,
      std::unordered_map<std::string, std::vector<std::vector<Position>>> const positions,
      std::unordered_map<std::string, std::vector<std::vector<Rotation>>> const rotations,
      std::unordered_map<std::string, std::vector<std::vector<Scale>>> const scales) {

    auto const& positionData = raw_data["positions"].at(0);

    glm::vec3 position{};
    position = glm::vec3(
      positionData["x"].template get<float>(),
      positionData["y"].template get<float>(),
      positionData["z"].template get<float>()
    );

    auto const& scale_data = raw_data["scales"].at(0);
    auto const& rotation_data = raw_data["rotations"].at(0);

    glm::vec3 const scale = glm::vec3(
      scale_data["x"].template get<float>(),
      scale_data["y"].template get<float>(),
      scale_data["z"].template get<float>()
    );
    glm::vec3 const rotation = glm::vec3(
      glm::radians(rotation_data["x"].template get<float>()),
      glm::radians(rotation_data["y"].template get<float>()),
      glm::radians(rotation_data["z"].template get<float>())
    );

    std::vector<std::string> textures{};

    if (raw_data.contains("textures")) {
      for (auto& [keyTex, pathTex] : raw_data["textures"].items()) {
        textures.emplace_back(static_cast<std::string>(keyTex));
      }
    }

    if (raw_data.contains("normal")) {
      for (auto& [keyTex, pathTex] : raw_data["normal"].items()) {
        textures.emplace_back(static_cast<std::string>(keyTex));
      }
    }

    if (raw_data.contains("mr")) {
      for (auto& [keyTex, pathTex] : raw_data["mr"].items()) {
        textures.emplace_back(static_cast<std::string>(keyTex));
      }
    }

    if (raw_data.contains("ao")) {
      for (auto& [keyTex, pathTex] : raw_data["emissive"].items()) {
        textures.emplace_back(static_cast<std::string>(keyTex));
      }
    }

    if (raw_data.contains("ao")) {
      for (auto& [keyTex, pathTex] : raw_data["ao"].items()) {
        textures.emplace_back(static_cast<std::string>(keyTex));
      }
    }

    if (raw_data.contains("transmission")) {
      for (auto& [keyTex, pathTex] : raw_data["transmission"].items()) {
        textures.emplace_back(static_cast<std::string>(keyTex));
      }
    }

    //bool const has_bbox = raw_data["hasBbox"].template get<bool>();
    bool const has_animation = raw_data["hasAnimation"].template get<bool>();
    //bool const is_point_light = raw_data["isPointLight"].template get<bool>();

    std::vector<std::string> animation_scripts{};
    animation_scripts.reserve(raw_data["animationScripts"].size());
    for (auto& [key_anim, path_anim] : raw_data["animationScripts"].items()) {
      animation_scripts.emplace_back(static_cast<std::string>(path_anim));
    }

    auto shader = raw_data["shader"].template get<std::string>();

    EntityOptions entity_opts = {
      shader, position, scale, glm::quat(rotation),
      raw_data["hasBbox"].template get<bool>(),
      raw_data["hasAnimation"].template get<bool>(),
      raw_data["isPointLight"].template get<bool>(),
      animation_scripts,
      raw_data["hasShadow"].template get<bool>(),
      raw_data["flipY"].template get<bool>(),
      raw_data["isIndexed"].template get<bool>(),
      raw_data["debugNormal"].template get<bool>()
    };


      std::unique_ptr<Mesh>mesh = std::make_unique<Mesh>();
      mesh->setName(_data.name);
      mesh->setShaderName(entity_opts.shader);
      mesh->setHasAnimation(entity_opts.has_animation);
      mesh->setIsPointLight(entity_opts.is_point_light);
      mesh->setHasShadow(entity_opts.has_shadow);
      mesh->setIsIndexed(entity_opts.is_indexed);
      //std::vector<Mesh::BBox> bboxs{};
      mesh->setDebugNormal(entity_opts.debug_normal);

      uint32_t const tex1ID = _data.materials_ID.at(0);

      std::string name_texture{ PLP_EMPTY };
      std::string name_specular_map{ PLP_EMPTY };
      std::string name_bump_map{ PLP_EMPTY };
      std::string name_alpha_map{ PLP_EMPTY };
      std::string name_texture_metal_roughness{ PLP_EMPTY };
      std::string name_texture_emissive{ PLP_EMPTY };
      std::string name_texture_ao{ PLP_EMPTY };
      std::string name_texture_base_color{ PLP_EMPTY };
      std::string name_texture_transmission{ PLP_EMPTY };
      float alpha_mode{ 0.0 };

      if (!materials.empty()) {

        auto const& mat = materials.at(_data.material_ID);
        alpha_mode = mat.alpha_mode;

        //@todo should not be in mesh, but just an ID pointing to the material
        mesh->setMaterial(mat);

        //@todo temp
        //@todo separate into 2 storage buffer of 3 texSample
        auto const& tex1 = materials.at(tex1ID);

        if (!tex1.name_texture_ambient.empty()) {
          name_texture = tex1.name_texture_ambient;
        } else if (!tex1.name_texture_diffuse.empty()) {
          name_texture = tex1.name_texture_diffuse;
        }


        if (!mat.name_texture_specular.empty()) {
          name_specular_map = mat.name_texture_specular;
        }

        if (!mat.name_texture_bump.empty()) {
          name_bump_map = mat.name_texture_bump;
        }

        if (!mat.name_texture_alpha.empty()) {
          name_alpha_map = mat.name_texture_alpha;
        }

        if (!mat.name_texture_metal_roughness.empty()) {
          name_texture_metal_roughness = mat.name_texture_metal_roughness;
        }
        if (!mat.name_texture_emissive.empty()) {
          name_texture_emissive = mat.name_texture_emissive;
        }
        if (!mat.name_texture_ao.empty()) {
          name_texture_ao = mat.name_texture_ao;
        }
        if (!mat.name_texture_base_color.empty()) {
          name_texture_base_color = mat.name_texture_base_color;
        }
        if (!mat.name_texture_transmission.empty()) {
          name_texture_transmission = mat.name_texture_transmission;
        }
      }

      Data data{};
      data._name = _data.name + '_' + name_texture;
      data._texture_prefix = _data.texture_prefix;
      data._textures.emplace_back(name_texture);
      data._specular_map = name_specular_map;
      data._bump_map = name_bump_map;
      data._alpha = name_alpha_map;
      data._metal_roughness = name_texture_metal_roughness;
      data._emissive = name_texture_emissive;
      data._ao = name_texture_ao;
      data._base_color = name_texture_base_color;
      data._transmission = name_texture_transmission;
      data._vertices = _data.vertices;
      data._indices = _data.indices;
      data._origin_pos = entity_opts.pos;
      data._current_pos = entity_opts.pos;
      data._origin_scale = entity_opts.scale;
      data._current_scale = entity_opts.scale;
      data._origin_rotation = entity_opts.rotation;
      data._current_rotation = entity_opts.rotation;
      data._inverse_transform_matrix = _data.inverse_transform_matrix;
      data._bones = _data.bones;
      data._root_bone_name = _data.root_bone_name;
      data._local_transform = _data.local_transform;

      glm::mat4 const S = glm::scale(glm::mat4(1.0f), entity_opts.scale);
      glm::mat4 const R = glm::toMat4(entity_opts.rotation);
      glm::mat4 const T = glm::translate(glm::mat4(1.0f), entity_opts.pos);
      glm::mat4 const transform = T * R * S;

      std::vector<std::vector<UniformBufferObject>> ubos{};
      UniformBufferObject ubo{};
      ubo.model = transform * data._local_transform;

      if (!data._bones.empty()) {
        auto const& root_bone = data._bones[data._root_bone_name];
        ubo.model = root_bone.t_pose * transform  ;
        ubos.reserve(data._bones.size());

        std::ranges::for_each(data._bones, [&](auto& bone) {
          auto const& b{ bone.second };
          std::vector<UniformBufferObject> tmp_ubos{ };
          tmp_ubos.resize(b.weights.size());

          std::fill(tmp_ubos.begin(), tmp_ubos.end(), ubo);
          ubos.push_back(tmp_ubos);
        });
      } else {
        ubos.push_back({ ubo });
      }

      data._transform_matrix = ubo.model;
      data._ubos = ubos;
      data._original_ubo = ubo;

      mesh->setData(data);
      //mesh->addBBox(box);

      bool const is_last{ (_data.id == 0) ? true : false };

      auto* entity = new Entity();
      entity->setName(_data.name);

      ComponentRenderingInfo rendering_info {
        .mesh = mesh.get(),
        .textures = _texture_manager->getTextures(),
        .skybox_name = _texture_manager->getSkyboxTexture(),
        .terrain_name = _texture_manager->getTerrainTexture(),
        .water_name = _texture_manager->getWaterTexture(),
        .sun_light = _light_manager->getSunLight(),
        .point_lights = _light_manager->getPointLights(),
        .spot_lights = _light_manager->getSpotLights(),
        .characters = {},
        .face = nullptr,
        .atlas_width = 0,
        .atlas_height = 0,
        .light_buffer = _light_buffer
      };

      auto basicRdrImpl { RendererComponentFactory::create<Basic>() };
      (*basicRdrImpl)(_renderer, rendering_info);

      _component_manager->add<RendererComponent>(entity->getID(), std::move(basicRdrImpl));
      _component_manager->add<MeshComponent>(entity->getID(), std::move(mesh));
      auto* entityNode = root_mesh_entity_node->addChild(new EntityNode(entity));

      //_renderer->addEntity(entityNode->getEntity(), is_last);

      if (alpha_mode == 2.0f) {
        addTransparentEntity(entityNode->getEntity());
      } else {
        addEntity(entityNode->getEntity());
      }

      if (is_last) {
        {
          std::lock_guard<std::shared_mutex> guard(lockWorldNode());
          //lua scripted animation
          if (has_animation) {
            //@todo temp until lua scripting
            for (auto& anim : entity_opts.animation_scripts) {
              auto animationScript = std::make_unique<AnimationScript>(anim);
              _component_manager->add<AnimationComponent>(entity->getID(), std::move(animationScript));
            }
          }
          //skeleton animation
          if (!animations.empty()) {
            auto boneAnimationScript = std::make_unique<BoneAnimationScript>(animations, positions, rotations, scales);
            _component_manager->add<BoneAnimationComponent>(
            entity->getID(), std::move(boneAnimationScript));
          }
          //std::shared_lock guard(_mutex_shared);
          root_mesh_entity_node->setIsLoaded(true);
          _world_node->addChild(root_mesh_entity_node);
        }
      }
    };
    AssimpLoader::loadData(path, flip_Y, callback);
  }

  void EntityManager::initWorldGraph()
  {
    Entity* _world = new Entity();
    _world->setName("_PLPWorld");
    _world->setVisible(false);

    _world_node = std::make_unique<EntityNode>(_world);
  }

  EntityNode const * EntityManager::addEntityToWorld(Entity * entity)
  {
    return _world_node->addChild(new EntityNode(entity));
  }

  void EntityManager::addEntity(Entity* entity)
  {
    _entities.emplace_back(entity);
  }

  void EntityManager::addTransparentEntity(Entity* entity)
  {
    _transparent_entities.emplace_back(entity);
  }

  void EntityManager::addTextEntity(Entity* entity)
  {
    _text_entities.emplace_back(entity);
  }
}
