module;

#include <tcl.h>
#include <tk.h>

#include <functional>
#include <iostream>
#include <latch>
#include <string>
#include <thread>

module Editor.Managers.EditorManager;

import Engine.Core.Logger;

import Engine.Component.Components;
import Engine.Managers.AudioManager;
import Engine.Managers.ComponentManager;
import Engine.Managers.DestroyManager;
import Engine.Managers.EntityManager;
import Engine.Managers.FontManager;
import Engine.Managers.LightManager;
import Engine.Managers.ShaderManager;
import Engine.Managers.TextureManager;
import Engine.Managers.RenderManager;
import Engine.Renderer.Mesh;
import Engine.Renderer.RendererComponent;
import Engine.Renderer.RendererComponentTypes;

namespace Poulpe {

  int test_callback(
    ClientData clientData,
    Tcl_Interp* interp,
    int argc,
    const char* argv[])
  {
    Tcl_SetResult(interp, const_cast<char*>("Hello from C++!"), TCL_STATIC);
    return TCL_OK;
  }

  int test_update_skybox(
    ClientData clientData,
    Tcl_Interp* interp,
    int argc,
    const char* argv[]
  ) {
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
      .mode = ComponentRenderingInfo::MODE::UPDATE
    };

    (*rdr_impl)(renderer, rendering_info);

    return TCL_OK;
  }
 
  EditorManager::EditorManager(
    RenderManager const * render_manager
  ) : _render_manager(render_manager)
  {
    _tcl_interp = Tcl_CreateInterp();

    if (Tcl_Init(_tcl_interp) != TCL_OK) {
      Logger::error("Could not init tcl_interp : {}", Tcl_GetStringResult(_tcl_interp));
      return;
    }
    if (Tk_Init(_tcl_interp) != TCL_OK) {
      Logger::error("Could not init tcl_interp : {}", Tcl_GetStringResult(_tcl_interp));
      return;
    }
    Logger::trace("Tcl Interpreted init");

    _init = true;

    Tcl_CreateCommand(_tcl_interp,
                  "test_callback",
                  test_callback,
                  const_cast<RenderManager*>(_render_manager),
                  nullptr);

    Tcl_CreateCommand(_tcl_interp,
                  "test_update_skybox",
                  test_update_skybox,
                  const_cast<RenderManager*>(_render_manager),
                  nullptr);

    if (Tcl_EvalFile(_tcl_interp, "./src/Editor/Scripts/main.tcl") != TCL_OK) {
      Logger::error("Could not init tcl_interp : {}", Tcl_GetStringResult(_tcl_interp));
      return;
    }

    Tk_MainLoop();
  }

  EditorManager::~EditorManager()
  {
    Tcl_DeleteInterp(_tcl_interp);
    Tcl_Finalize();
  }
}
