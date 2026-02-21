export module Engine.Managers.EntityManager;

import std;

import Engine.Component.Entity;
import Engine.Component.EntityNode;

import Engine.Core.AnimationTypes;
import Engine.Core.Json;
import Engine.Core.MeshTypes;
import Engine.Core.GLM;

import Engine.Managers.ComponentManager;
import Engine.Managers.LightManager;
import Engine.Managers.TextureManager;

import Engine.Renderer;
import Engine.Renderer.RendererComponentFactory;

import Engine.Utils.IDHelper;

namespace Poulpe
{
  export class EntityManager
  {
  public:

    explicit EntityManager(
      ComponentManager& component_manager,
      TextureManager& texture_manager);

    ~EntityManager() = default;

    void clear();

    inline std::shared_ptr<Entity> getSkybox() const { return _skybox; }
    inline std::shared_ptr<Entity> getTerrain() const { return _terrain; }
    inline std::shared_ptr<Entity> getWater() const { return _water; }
    inline std::shared_ptr<Entity> getShadowMap() const { return _shadow_map; }
    inline std::shared_ptr<Entity> getText(std::uint32_t const index) const { return _texts.at(index); }

    inline std::span<std::shared_ptr<Entity>> getTexts() { return _texts; }
    std::span<std::shared_ptr<Entity>> getEntities() { return _entities;}
    std::span<std::shared_ptr<Entity>> getTransparentEntities() { return _transparent_entities;}

    std::function<void()> load(Renderer const& renderer, json const& lvl_config);

    inline void setSkybox(std::shared_ptr<Entity> skybox) { _skybox = skybox; }
    inline void setTerrain(std::shared_ptr<Entity> terrain) { _terrain = terrain; }
    inline void setWater(std::shared_ptr<Entity> water) { _water = water; }
    inline void setShadowMap(std::shared_ptr<Entity> shadow_map) { _shadow_map = shadow_map; }

    inline std::size_t addText(std::shared_ptr<Entity> text) { _texts.emplace_back(std::move(text)); return _texts.size() - 1; }

    void addEntity(std::shared_ptr<Entity> entity);
    void addTransparentEntity(std::shared_ptr<Entity> entity);
    void addTextEntity(std::shared_ptr<Entity> entity);

    std::shared_ptr<EntityNode> const addEntityToWorld(std::shared_ptr<Entity> entity);

    EntityNode * getWorldNode();

    void initMeshes(
      Renderer const& renderer,
      std::string const& name,
      json const& raw_data);

    void initWorldGraph();
    std::shared_mutex& lockWorldNode() { return _mutex_shared; }

  private:
    ComponentManager& _component_manager;
    TextureManager& _texture_manager;

    json _lvl_config;

    std::shared_ptr<Entity> _skybox{nullptr};
    std::shared_ptr<Entity> _terrain{nullptr};
    std::shared_ptr<Entity> _water{nullptr};
    std::shared_ptr<Entity> _shadow_map{nullptr};

    std::vector<std::shared_ptr<Entity>> _texts;

    std::vector<std::shared_ptr<Entity>> _entities{};
    std::vector<std::shared_ptr<Entity>> _transparent_entities{};
    std::vector<std::shared_ptr<Entity>> _text_entities{};

    std::shared_ptr<EntityNode> _world_node;

    mutable std::shared_mutex _mutex_shared;

    std::unordered_map<std::size_t, std::unordered_map<std::string, std::vector<std::vector<Rotation>>>> _rotations{};
    std::unordered_map<std::size_t, std::unordered_map<std::string, std::vector<std::vector<Position>>>> _positions{};
    std::unordered_map<std::size_t, std::unordered_map<std::string, std::vector<std::vector<Scale>>>> _scales{};
    std::unordered_map<std::size_t, std::vector<Animation>> _animations{};
    std::vector<IDType> _entity_children{};
  };
}
