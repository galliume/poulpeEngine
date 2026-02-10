module Editor.Managers.ExplorerManager;

import std;

import Engine.Core.Logger;
import Engine.Core.TclTk;

import Engine.Component.Components;

import Engine.Managers.AudioManager;
import Engine.Managers.TextureManager;
import Engine.Managers.ConfigManagerLocator;

namespace Poulpe {
  static int plp_list_ambient_sound (
    ClientData,
    Tcl_Interp* interp,
    int,
    Tcl_Obj *const [])
  {
    //auto * render_manager = static_cast<RenderManager*>(client_data);
    auto const * const config_manager { ConfigManagerLocator::get() };
    auto const sounds { config_manager->soundConfig()["ambient"] };

    Tcl_Obj* sounds_list = Tcl_NewListObj(0, nullptr);
    Tcl_ListObjAppendElement(interp, sounds_list, Tcl_NewStringObj(".c.ambient.ambient_sounds", -1));
    Tcl_ListObjAppendElement(interp, sounds_list, Tcl_NewStringObj("insert", -1));
    Tcl_ListObjAppendElement(interp, sounds_list, Tcl_NewStringObj("end", -1));

    for (auto const& sound : sounds) {
      Tcl_ListObjAppendElement(interp, sounds_list, Tcl_NewStringObj(sound.get<std::string>().c_str(), -1));
    }
    Tcl_IncrRefCount(sounds_list);
    if (Tcl_EvalObjEx(interp, sounds_list, TCL_EVAL_DIRECT) != TCL_OK) {
      std::cerr << "Tcl Error: " << Tcl_GetStringResult(interp) << std::endl;
    }
    Tcl_DecrRefCount(sounds_list);
    return TCL_OK;
  }

  static int plp_play_sound(
    ClientData client_data,
    Tcl_Interp* interp,
    int,
    Tcl_Obj *const objv[])
  {
    auto * const render_manager { static_cast<RenderManager*>(client_data) };
    auto * const audio_manager { render_manager->getAudioManager() };
    //auto * const config_manager { ConfigManagerLocator::get() };

    std::uint32_t sound_index{0};
    Tcl_GetIntFromObj(interp, objv[1], (int*)&sound_index);

    audio_manager->startAmbient(sound_index);

    return TCL_OK;
  }

  void ExplorerManager::registerCommand(
    Tcl_Interp * tcl_interp,
    RenderManager const * render_manager)
    {
      Tcl_CreateObjCommand(
        tcl_interp,
        "plp_list_ambient_sound",
        plp_list_ambient_sound,
        const_cast<RenderManager*>(render_manager),
        nullptr);

      Tcl_CreateObjCommand(
        tcl_interp,
        "plp_play_sound",
        plp_play_sound,
        const_cast<RenderManager*>(render_manager),
        nullptr);
    }
}
