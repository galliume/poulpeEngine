export module Engine.Managers.InputManager;

import std;

import Engine.Core.Camera;
import Engine.Core.GLFW;
import Engine.Core.Json;

import Engine.GUI.Window;

import Engine.Managers.PlayerManager;

namespace Poulpe
{
  export class InputManager
  {
  public:
    explicit InputManager(Window const * const window);

    void init(json const& input_config) ;

    void keyPress(int key, int scan_code, int action, int mods);
    void mouseButton(int button, int action, int mods);
    void saveLastMousePos(double x_pos, double y_pos);
    void inline setCamera(Camera * const camera) { _camera = camera; }
    void updateMousePos(double x_pos, double y_pos);
    void processGamepad(PlayerManager * player_manager, double const delta_time);

    static bool _can_move_camera;
    static bool _first_move_mouse;

  private:
    double _last_x{ 0 };
    double _last_y{ 0 };

    //@todo clean this
    Camera * _camera;
    Window const * const _window;

    json _input_config;
    std::unordered_map<std::string, int> _keyboard_keys{};

    bool _has_gamepad { false };
  };
}
