module;
#include "GLFW/glfw3.h"
#include "nlohmann/json.hpp"

module Engine.Managers.InputManager;

import Engine.Core.Logger;

import Engine.GUI.Window;

import Engine.Managers.ConfigManagerLocator;
import Engine.Managers.InputManagerLocator;

namespace Poulpe
{
  bool InputManager::_can_move_camera = false;
  bool InputManager::_first_move_mouse = true;

  InputManager::InputManager(Window const * const window) : _window(window)
  {
    int width, height;
    glfwGetWindowSize(_window->getGlfwWindow(), & width, & height);
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

    glfwSetWindowUserPointer(_window->getGlfwWindow(), this);

    glfwSetKeyCallback(_window->getGlfwWindow(), [](GLFWwindow*, int key, int scan_code, int action, int mods) {
      InputManager* input_manager = InputManagerLocator::get();
      input_manager->keyPress(key, scan_code, action, mods);
    });

    glfwSetCursorPosCallback(_window->getGlfwWindow(), []( GLFWwindow*, double x_pos, double y_pos) {
      InputManager* input_manager = InputManagerLocator::get();
      input_manager->updateMousePos(x_pos, y_pos);
    });

    glfwSetMouseButtonCallback(_window->getGlfwWindow(), []( GLFWwindow*, int button, int action, int mods) {
      InputManager* input_manager = InputManagerLocator::get();
      input_manager->mouseButton(button, action, mods);
    });

    int present { glfwJoystickIsGamepad(GLFW_JOYSTICK_1) };
    if (present) {
      _has_gamepad = true;
      Logger::debug("GLFW_JOYSTICK_1: {}",  glfwGetGamepadName(GLFW_JOYSTICK_1));
    }
  }

  void InputManager::keyPress(
    int const key,
    int const,
    int const action,
    int const)
  {
    const auto config = _input_config[_input_config["current"]];

    switch (action)
    {
      case GLFW_PRESS:
      case GLFW_REPEAT:
      {
        if (key == _keyboard_keys[config["unlockCamera"]]) {
          if (!InputManager::_can_move_camera) {
            glfwSetInputMode(_window->getGlfwWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
          } else {
            glfwSetInputMode(_window->getGlfwWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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
          ConfigManagerLocator::get()->updateConfig<uint32_t>("fpsLimit", 0);
        } else if (key == _keyboard_keys[config["reloadShader"]])
        {
          ConfigManagerLocator::get()->setReloadShaders(true);
        } else if (key == _keyboard_keys[config["normalDebug"]])
        {
          ConfigManagerLocator::get()->setNormalDebug();
        } else if (key == _keyboard_keys[config["reload"]])
        {
          ConfigManagerLocator::get()->setReload(true);
        }
        else if (key == _keyboard_keys[config["switch_camera"]])
        {
          ConfigManagerLocator::get()->switchCamera();
        }

        break;
      }
      case GLFW_RELEASE:
      {
        //nothing, yet.
        break;
      }
      default:
      //nothing, yet.
      break;
    }
  }

  void InputManager::mouseButton(
    int const button,
    int const,
    int const)
  {
    if (GLFW_MOUSE_BUTTON_LEFT == button) {
      int width, height;
      glfwGetWindowSize(_window->getGlfwWindow(), &width, &height);

      double x_pos, y_pos;
      glfwGetCursorPos(_window->getGlfwWindow(), &x_pos, &y_pos);
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

    const double sensitivity = 0.05;

    x_offset *= sensitivity;
    y_offset *= sensitivity;

    _camera->updateAngle(x_offset, y_offset);
  }

  void InputManager::processGamepad(PlayerManager * player_manager, double const delta_time)
  {
    if (!_has_gamepad) {
      return;
    }

    // int count;
    // const unsigned char* hats = glfwGetJoystickHats(GLFW_JOYSTICK_1, &count);
    // if (hats[2] & GLFW_HAT_RIGHT)
    // {
    //   Logger::debug("right");
    // }

    GLFWgamepadstate state;
    if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state))
    {
      if (state.buttons[GLFW_GAMEPAD_BUTTON_A])
      {
        player_manager->jump();
      }

      float lx { state.axes[GLFW_GAMEPAD_AXIS_LEFT_X] };
      float ly { state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] };
      float rx { state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X] };
      float ry { state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] };
      float const deadzone { 0.2f };

      if (std::abs(lx) < deadzone) lx = 0.0f;
      if (std::abs(ly) < deadzone) ly = 0.0f;

      if (std::abs(rx) < deadzone) rx = 0.0f;
      if (std::abs(ry) < deadzone) ry = 0.0f;

      if (lx != 0.0f || ly != 0.0f) {
        player_manager->move(lx, ly, delta_time);
      }
      if (rx != 0.0f || ry != 0.0f) {
        auto pos {_camera->getPos()};
        pos.z -= 100;
        _camera->setPos(pos);
        InputManager::_can_move_camera = true;
        updateMousePos(static_cast<double>(rx) * delta_time, static_cast<double>(ry) * delta_time);
      }
    }
  }
}
