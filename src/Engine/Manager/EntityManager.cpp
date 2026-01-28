module;

//@todo find a way to encapsulate this in json.cppm
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

module Engine.Managers.EntityManager;

import std;

import Engine.Animation.AnimationScript;
import Engine.Animation.BoneAnimationScript;

import Engine.Component.Components;
import Engine.Component.Entity;
import Engine.Component.EntityNode;
import Engine.Component.Mesh;

import Engine.Core.AssimpLoader;
import Engine.Core.AnimationTypes;
import Engine.Core.Constants;
import Engine.Core.Json;
import Engine.Core.Logger;
import Engine.Core.MaterialTypes;
import Engine.Core.GLM;
import Engine.Core.MeshTypes;
import Engine.Core.PlpTypedef;
import Engine.Core.Volk;

import Engine.Managers.ComponentManager;
import Engine.Managers.ConfigManagerLocator;

import Engine.Renderer;
import Engine.Renderer.RendererComponent;
import Engine.Renderer.RendererComponentFactory;
import Engine.Renderer.RendererComponentTypes;

import Engine.Renderer.Vulkan.Basic;

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

  std::function<void()> EntityManager::load(json const& lvl_config)
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

  void EntityManager::initMeshes(std::string const& name, json const& raw_data)
  {
    //std::vector<Mesh*> meshes{};
    //if (_ObjLoaded.contains(path)) return meshes;

    //_ObjLoaded.insert(path);

    //@todo not reload an already loaded obj
    auto root_mesh_entity { std::make_shared<Entity>() };
    root_mesh_entity->setName(name);
    root_mesh_entity->setVisible(false);

    auto root_mesh_entity_node { std::make_shared<EntityNode>(root_mesh_entity) };

    auto const root_path { ConfigManagerLocator::get()->rootPath() };

    auto const& path { root_path + "/" + raw_data.value("mesh", "") };
    auto const flip_Y { raw_data.value("flipY", false) };

    auto callback = [&](
      PlpMeshData _data,
      std::vector<material_t> const materials,
      std::vector<Animation> const animations,
      std::unordered_map<std::string, std::vector<std::vector<Position>>> const positions,
      std::unordered_map<std::string, std::vector<std::vector<Rotation>>> const rotations,
      std::unordered_map<std::string, std::vector<std::vector<Scale>>> const scales) {

    auto const& p = raw_data["positions"].at(0);
    glm::vec3 position{ p["x"].get<float>(), p["y"].get<float>(), p["z"].get<float>() };
    
    auto const& s = raw_data["scales"].at(0);
    glm::vec3 scale{ s["x"].get<float>(), s["y"].get<float>(), s["z"].get<float>() };

    auto const& r = raw_data["rotations"].at(0);
    glm::quat rotation { glm::quat(glm::vec3(glm::radians(r["x"].get<float>()),
                                  glm::radians(r["y"].get<float>()),
                                  glm::radians(r["z"].get<float>()))) };

    std::vector<std::string> textures{};

    if (raw_data.contains("textures")) {
      for (auto& [keyTex, pathTex] : raw_data["textures"].items()) {
        textures.emplace_back(keyTex);
      }
    }

    if (raw_data.contains("normal")) {
      for (auto& [keyTex, pathTex] : raw_data["normal"].items()) {
        textures.emplace_back(keyTex);
      }
    }

    if (raw_data.contains("mr")) {
      for (auto& [keyTex, pathTex] : raw_data["mr"].items()) {
        textures.emplace_back(keyTex);
      }
    }

    if (raw_data.contains("emissive")) {
      for (auto& [keyTex, pathTex] : raw_data["emissive"].items()) {
        textures.emplace_back(keyTex);
      }
    }

    if (raw_data.contains("ao")) {
      for (auto& [keyTex, pathTex] : raw_data["ao"].items()) {
        textures.emplace_back(keyTex);
      }
    }

    if (raw_data.contains("transmission")) {
      for (auto& [keyTex, pathTex] : raw_data["transmission"].items()) {
        textures.emplace_back(keyTex);
      }
    }

    std::vector<std::string> animation_scripts{};
    animation_scripts.reserve(raw_data["animationScripts"].size());
    for (auto& [key_anim, path_anim] : raw_data["animationScripts"].items()) {
      animation_scripts.emplace_back(path_anim);
    }

    auto shader = raw_data.value("shader", "");

    EntityOptions entity_opts = {
      shader, position, scale, rotation,
      raw_data.value("hasBbox", false),
      raw_data.value("hasAnimation", false),
      raw_data.value("isPointLight", false),
      animation_scripts,
      raw_data.value("hasShadow", false),
      raw_data.value("flipY", false),
      raw_data.value("isIndexed", false),
      raw_data.value("debugNormal", false),
      raw_data.value("defaultAnim", 0u)
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

      std::string name_texture { PLP_EMPTY };
      std::string name_specular_map { PLP_EMPTY };
      std::string name_bump_map { PLP_EMPTY };
      std::string name_alpha_map { PLP_EMPTY };
      std::string name_texture_metal_roughness { PLP_EMPTY };
      std::string name_texture_emissive { PLP_EMPTY };
      std::string name_texture_ao { PLP_EMPTY };
      std::string name_texture_base_color { PLP_EMPTY };
      std::string name_texture_transmission { PLP_EMPTY };
      float alpha_mode{ 0.0 };

      std::uint32_t options {0};

      if (!materials.empty()) {

        auto const& mat = materials.at(_data.material_ID);
        alpha_mode = mat.alpha_mode;

        if (!mat.name_texture_diffuse.empty()) {
          name_texture = mat.name_texture_diffuse;
          _texture_manager->add(name_texture, mat.name_texture_diffuse_path, VK_IMAGE_ASPECT_COLOR_BIT, TEXTURE_TYPE::DIFFUSE, _renderer);
          options |= PLP_MESH_OPTIONS::HAS_BASE_COLOR;
        } else if (!mat.name_texture_ambient.empty()) {
          name_texture = mat.name_texture_ambient;
          options |= PLP_MESH_OPTIONS::HAS_BASE_COLOR;
          _texture_manager->add(name_texture, mat.name_texture_ambient_path, VK_IMAGE_ASPECT_COLOR_BIT, TEXTURE_TYPE::DIFFUSE, _renderer);
        }

        if (!mat.name_texture_specular.empty()) {
          name_specular_map = mat.name_texture_specular;
          options |= PLP_MESH_OPTIONS::HAS_SPECULAR;
          _texture_manager->add(name_specular_map, mat.name_texture_specular_path, VK_IMAGE_ASPECT_COLOR_BIT, TEXTURE_TYPE::AO, _renderer);
        }

        if (!mat.name_texture_bump.empty()) {
          name_bump_map = mat.name_texture_bump;
          options |= PLP_MESH_OPTIONS::HAS_NORMAL;
          _texture_manager->add(name_bump_map, mat.name_texture_bump_path, VK_IMAGE_ASPECT_COLOR_BIT, TEXTURE_TYPE::NORMAL, _renderer);
        }

        if (!mat.name_texture_alpha.empty()) {
          name_alpha_map = mat.name_texture_alpha;
          options |= PLP_MESH_OPTIONS::HAS_ALPHA;
          _texture_manager->add(name_alpha_map, mat.name_texture_alpha_path, VK_IMAGE_ASPECT_COLOR_BIT, TEXTURE_TYPE::AO, _renderer);
        }

        if (!mat.name_texture_metal_roughness.empty()) {
          name_texture_metal_roughness = mat.name_texture_metal_roughness;
          options |= PLP_MESH_OPTIONS::HAS_MR;
          _texture_manager->add(name_texture_metal_roughness, mat.name_texture_metal_roughness_path, VK_IMAGE_ASPECT_COLOR_BIT, TEXTURE_TYPE::MR, _renderer);
        }

        if (!mat.name_texture_emissive.empty()) {
          name_texture_emissive = mat.name_texture_emissive;
          options |= PLP_MESH_OPTIONS::HAS_EMISSIVE;
          _texture_manager->add(name_texture_emissive, mat.name_texture_emissive_path, VK_IMAGE_ASPECT_COLOR_BIT, TEXTURE_TYPE::EMISSIVE, _renderer);
        }

        if (!mat.name_texture_ao.empty()) {
          name_texture_ao = mat.name_texture_ao;
          options |= PLP_MESH_OPTIONS::HAS_AO;
          _texture_manager->add(name_texture_ao, mat.name_texture_ao_path, VK_IMAGE_ASPECT_COLOR_BIT, TEXTURE_TYPE::AO, _renderer);
        }

        if (!mat.name_texture_base_color.empty()) {
          name_texture_base_color = mat.name_texture_base_color;
          options |= PLP_MESH_OPTIONS::HAS_BASE_COLOR;
          _texture_manager->add(name_texture_base_color, mat.name_texture_base_color_path, VK_IMAGE_ASPECT_COLOR_BIT, TEXTURE_TYPE::DIFFUSE, _renderer);
        }

        if (!mat.name_texture_transmission.empty()) {
          name_texture_transmission = mat.name_texture_transmission;
          options |= PLP_MESH_OPTIONS::HAS_TRANSMISSION;
          _texture_manager->add(name_texture_transmission, mat.name_texture_transmission_path, VK_IMAGE_ASPECT_COLOR_BIT, TEXTURE_TYPE::EMISSIVE, _renderer);
        }
        for (auto& material : materials) {
          mesh->addMaterial(material);
        }
      }

      Data data{};
      data._name = _data.name;
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
      data._vertices = std::move(_data.vertices);
      data._vertices_bones = std::move(_data.vertices_bones);
      data._indices = _data.indices;
      data._origin_pos = entity_opts.pos;
      data._current_pos = entity_opts.pos;
      data._origin_scale = entity_opts.scale;
      data._current_scale = entity_opts.scale;
      data._origin_rotation = entity_opts.rotation;
      data._current_rotation = entity_opts.rotation;
      data._bones = _data.bones;
      data._root_bone_name = _data.root_bone_name;
      data._local_transform = _data.local_transform;
      data._default_anim = entity_opts.default_anim;
      data._bbox_min = _data.bbox_min;
      data._bbox_max = _data.bbox_max;
      data._material_id = _data.material_ID;

      auto const S { glm::scale(glm::mat4(1.0f), entity_opts.scale) };
      auto const R { glm::mat4_cast(entity_opts.rotation) };
      auto const T { glm::translate(glm::mat4(1.0f), entity_opts.pos) };
      auto const transform  { T * R * S };
      
      std::vector<std::vector<UniformBufferObject>> ubos{};
      UniformBufferObject ubo{};
      ubo.model = transform * _data.inverse_transform_matrix * _data.transform_matrix;

      data._inverse_transform_matrix = _data.inverse_transform_matrix;

      // if (!data._bones.empty()) {
      //   ubos.reserve(data._bones.size());
      //   std::ranges::for_each(data._bones, [&](auto& bone) {
      //     auto const& b{ bone.second };
      //     std::vector<UniformBufferObject> tmp_ubos{ };
      //     tmp_ubos.resize(b.weights.size());

      //     std::fill(tmp_ubos.begin(), tmp_ubos.end(), ubo);
      //     ubos.push_back(tmp_ubos);
      //   });
      // } else {
        ubos.push_back({ ubo });
      //}

      data._transform_matrix = _data.transform_matrix;
      data._ubos = ubos;
      data._original_ubo = ubo;

      mesh->bbox(true);
      mesh->setOptions(options);
      bool const is_last{ (_data.id == 0) ? true : false };
      
      auto entity { std::make_shared<Entity>() };
      entity->setName(_data.name);
      
      if (is_last) {
        mesh->setRoot();
        for (auto child : _entity_children) {
          mesh->addChild(child);
        }
      } else {
        _entity_children.emplace_back(entity->getID());
      }
      data._id = entity->getID();
      mesh->setData(data);

      _animations[data._id] = animations;
      _rotations [data._id] = rotations;
      _positions[data._id] = positions;
      _scales[data._id] = scales;

      auto basicRdrImpl { RendererComponentFactory::create<Basic>() };

      _component_manager->add<RendererComponent>(entity->getID(), std::move(basicRdrImpl));
      _component_manager->add<MeshComponent>(entity->getID(), std::move(mesh));
      auto entityNode { root_mesh_entity_node->addChild(std::make_shared<EntityNode>(entity)) };

      //_renderer->addEntity(entityNode->getEntity(), is_last);

      if (alpha_mode == 2.0f) {
        addTransparentEntity(entityNode->getEntity());
      } else {
        addEntity(entityNode->getEntity());
      }

      if (is_last) {
        {
          //lua scripted animation  
          if (entity_opts.has_animation) {
            //@todo temp until lua scripting
            for (auto& anim : entity_opts.animation_scripts) {
              auto animationScript = std::make_unique<AnimationScript>(anim);
              _component_manager->add<AnimationComponent>(entity->getID(), std::move(animationScript));
            }
          }

          //skeleton animation
          if (!animations.empty()) {
            auto boneAnimationScript = std::make_unique<BoneAnimationScript>(
              std::move(_animations),
              std::move(_positions),
              std::move(_rotations),
              std::move(_scales),
              entity_opts.default_anim);

            _component_manager->add<BoneAnimationComponent>(
            entity->getID(), std::move(boneAnimationScript));
            _entity_children.clear();
            _animations.clear();
            _rotations.clear();
            _positions.clear();
            _scales.clear();
          }

          std::lock_guard<std::shared_mutex> guard(lockWorldNode());
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
    auto _world { std::make_shared<Entity>() };
    _world->setName("_PLPWorld");
    _world->setVisible(false);

    _world_node = std::make_shared<EntityNode>(_world);
  }

  std::shared_ptr<EntityNode> const EntityManager::addEntityToWorld(std::shared_ptr<Entity> entity)
  {
    return _world_node->addChild(std::make_shared<EntityNode>(entity));
  }

  void EntityManager::addEntity(std::shared_ptr<Entity> entity)
  {
    _entities.emplace_back(entity);
  }

  void EntityManager::addTransparentEntity(std::shared_ptr<Entity> entity)
  {
    _transparent_entities.emplace_back(entity);
  }

  void EntityManager::addTextEntity(std::shared_ptr<Entity> entity)
  {
    _text_entities.emplace_back(entity);
  }
}
