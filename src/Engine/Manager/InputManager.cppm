module;

#include <GLFW/glfw3.h>
#include <nlohmann/json.hpp>

#include <unordered_map>
#include <string>

export module Engine.Managers.InputManager;

import Engine.Component.Camera;
import Engine.GUI.Window;

namespace Poulpe
{
  export class InputManager
  {
  public:
    explicit InputManager(Window const * const window);

    void init(nlohmann::json const& input_config) ;

    void keyPress(int key, int scan_code, int action, int mods);
    void mouseButton(int button, int action, int mods);
    void saveLastMousePos(double x_pos, double y_pos);
    void inline setCamera(Camera * const camera) { _camera = camera; }
    void updateMousePos(double x_pos, double y_pos);

    static bool _can_move_camera;
    static bool _first_move_mouse;

  private:
    double _last_x{ 0 };
    double _last_y{ 0 };

    //@todo clean this
    Camera * _camera;
    Window const * const _window;

    nlohmann::json _input_config;
    std::unordered_map<std::string, int> _keyboard_keys{};
  };
}
