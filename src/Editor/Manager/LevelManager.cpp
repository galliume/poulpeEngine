module Editor.Managers.LevelManager;

import std;

import Engine.Core.Logger;
import Engine.Core.TclTk;

import Engine.Component.Components;
import Engine.Component.Mesh;

import Engine.Managers.AudioManager;
import Engine.Managers.ComponentManager;
import Engine.Managers.DestroyManager;
import Engine.Managers.EntityManager;
import Engine.Managers.FontManager;
import Engine.Managers.LightManager;
import Engine.Managers.ShaderManager;
import Engine.Managers.TextureManager;
import Engine.Managers.RenderManager;

import Engine.Renderer.RendererComponent;
import Engine.Renderer.RendererComponentTypes;

namespace Poulpe {
  static int plp_update_skybox(
    ClientData clientData,
    Tcl_Interp*,
    int,
    Tcl_Obj *const [])
  {
    auto * render_manager = static_cast<RenderManager*>(clientData);
    auto * texture_manager = render_manager->getTextureManager();
    auto * light_manager = render_manager->getLightManager();
    auto * font_manager = render_manager->getFontManager();
    auto * entity_manager = render_manager->getEntityManager();
    auto * component_manager = render_manager->getComponentManager();
    auto * renderer = render_manager->getRenderer();

    std::latch count_down{ 1 };
    texture_manager->loadSkybox("bluesky", renderer)(count_down);
    count_down.wait();

    auto skybox = entity_manager->getSkybox();
    auto* mesh_component = component_manager->get<MeshComponent>(skybox->getID());
    auto* rdr_impl = component_manager->get<RendererComponent>(skybox->getID());
    if (!mesh_component) return TCL_OK;

    auto* mesh = mesh_component->has<Mesh>();
    if (!mesh) return TCL_OK;

    mesh->setIsDirty(true);
    mesh->getData()->_texture_index = 1;

    ComponentRenderingInfo rendering_info {
      .mesh = mesh,
      .textures = texture_manager->getTextures(),
      .skybox_name = texture_manager->getSkyboxTexture(),
      .terrain_name = texture_manager->getTerrainTexture(),
      .water_name = texture_manager->getWaterTexture(),
      .sun_light = light_manager->getSunLight(),
      .point_lights = light_manager->getPointLights(),
      .spot_lights = light_manager->getSpotLights(),
      .characters = font_manager->getCharacters(),
      .face = font_manager->getFace(),
      .atlas_width = font_manager->getAtlasWidth(),
      .atlas_height = font_manager->getAtlasHeight(),
      .mode = ComponentRenderingInfo::MODE::UPDATE,
      .light_buffer = render_manager->getLightBuffer()
    };

    (*rdr_impl)(renderer, rendering_info);

    return TCL_OK;
  }

  void LevelManager::registerCommand(
    Tcl_Interp * tcl_interp,
    RenderManager const * render_manager)
    {
      Tcl_CreateObjCommand(
        tcl_interp,
        "plp_update_skybox",
        plp_update_skybox,
        const_cast<RenderManager*>(render_manager),
        nullptr);
    }
}
