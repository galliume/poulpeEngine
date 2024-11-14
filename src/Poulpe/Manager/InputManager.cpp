#include "InputManager.hpp"

namespace Poulpe
{
  bool InputManager::_can_move_camera = false;
  bool InputManager::_first_move_mouse = true;

  InputManager::InputManager(Window const * const window) : _window(window)
  {
    int width, height;
    glfwGetWindowSize(_window->get(), & width, & height);
  }
    
  void InputManager::init(nlohmann::json const& input_config)
  {
    _input_config = input_config;

    //@todo should not be here and take keyboard type (qwerty, azerty, etc).
    _keyboard_keys = {
      { "left_ctrl", GLFW_KEY_LEFT_CONTROL },
      { "a", GLFW_KEY_A },
      { "b", GLFW_KEY_B },
      { "c", GLFW_KEY_C },
      { "d", GLFW_KEY_D },
      { "e", GLFW_KEY_E },
      { "f", GLFW_KEY_F },
      { "g", GLFW_KEY_G },
      { "g", GLFW_KEY_H },
      { "i", GLFW_KEY_I },
      { "j", GLFW_KEY_J },
      { "k", GLFW_KEY_K },
      { "l", GLFW_KEY_L },
      { "m", GLFW_KEY_M },
      { "n", GLFW_KEY_N },
      { "o", GLFW_KEY_O },
      { "p", GLFW_KEY_P },
      { "q", GLFW_KEY_Q },
      { "r", GLFW_KEY_R },
      { "s", GLFW_KEY_S },
      { "t", GLFW_KEY_T },
      { "u", GLFW_KEY_U },
      { "v", GLFW_KEY_V },
      { "w", GLFW_KEY_W },
      { "x", GLFW_KEY_X },
      { "y", GLFW_KEY_Y },
      { "z", GLFW_KEY_Z },
      { "0", GLFW_KEY_0 },
      { "1", GLFW_KEY_1 },
      { "2", GLFW_KEY_2 },
      { "3", GLFW_KEY_3 },
      { "4", GLFW_KEY_4 },
      { "5", GLFW_KEY_5 },
      { "6", GLFW_KEY_6 },
      { "7", GLFW_KEY_7 },
      { "8", GLFW_KEY_8 },
      { "9", GLFW_KEY_9 },
    };

    glfwSetWindowUserPointer(_window->get(), this);

    glfwSetKeyCallback(_window->get(), []( GLFWwindow* window, int key, int scan_code, int action, int mods) {
      InputManager* input_manager = Locator::getInputManager();
      input_manager->key(key, scan_code, action, mods);
    });

    glfwSetCursorPosCallback(_window->get(), []( GLFWwindow* window, double x_pos, double y_pos) {
      InputManager* input_manager = Locator::getInputManager();
      input_manager->updateMousePos(x_pos, y_pos);
    });

    glfwSetMouseButtonCallback(_window->get(), []( GLFWwindow* window, int button, int action, int mods) {
      InputManager* input_manager = Locator::getInputManager();
      input_manager->mouseButton(button, action, mods);
    });
  }

  void InputManager::key(
    int const key,
    int const scan_code,
    int const action,
    int const mods)
  {
    const auto config = _input_config[_input_config["current"]];

    switch (action)
    {
      case GLFW_PRESS:
      case GLFW_REPEAT:
      {
        if (key == _keyboard_keys[config["unlockCamera"]]) {
          if (!InputManager::_can_move_camera) {
            glfwSetInputMode(_window->get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
          } else {
            glfwSetInputMode(_window->get(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
          }
          InputManager::_can_move_camera = !InputManager::_can_move_camera;
        } else if (key == _keyboard_keys[config["forward"]]) {
          _camera->forward();
        } else if (key == _keyboard_keys[config["backward"]]) {
          _camera->backward();
        } else if (key == _keyboard_keys[config["left"]]) {
          _camera->left();
        } else if (key == _keyboard_keys[config["right"]]) {
          _camera->right();
        } else if (key == _keyboard_keys[config["up"]]) {
          _camera->up();
        } else if (key == _keyboard_keys[config["down"]]) {
          _camera->down();
        } else if (key == _keyboard_keys[config["unlockFPS"]])
        {
          Poulpe::Locator::getConfigManager()->updateConfig<unsigned int>("fpsLimit", 0);
        }
        break;
      }
      case GLFW_RELEASE:
      {
        //nothing, yet.
      }
    }
  }

  void InputManager::mouseButton(
    int const button,
    int const action,
    int const mods)
  {
    if (GLFW_MOUSE_BUTTON_LEFT == button) {
      int width, height;
      glfwGetWindowSize(_window->get(), &width, &height);

      double x_pos, y_pos;
      glfwGetCursorPos(_window->get(), &x_pos, &y_pos);
    }
  }

  void InputManager::saveLastMousePos(double const x_pos, double const y_pos)
  {
    _last_x = x_pos;
    _last_y = y_pos;

    InputManager::_first_move_mouse = true;
  }

  void InputManager::updateMousePos(double const x, double const y)
  {
    double x_pos = x;
    double y_pos = y;

    if (!InputManager::_can_move_camera) return;

    if (InputManager::_first_move_mouse) {
      _last_x = x_pos;
      _last_y = y_pos;
      InputManager::_first_move_mouse = false;
    }

    double x_offset = x_pos - _last_x;
    double y_offset = _last_y - y_pos;

    _last_x = x_pos;
    _last_y = y_pos;

    const double sensitivity = 0.5;

    x_offset *= sensitivity;
    y_offset *= sensitivity;

    _camera->updatePos(x_offset, y_offset);
  }
}
