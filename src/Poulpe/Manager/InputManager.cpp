#include "InputManager.hpp"


namespace Poulpe
{
    bool InputManager::_CanMoveCamera = false;
    bool InputManager::_FirtMouseMove = true;

    InputManager::InputManager(Window const * const window) : _window(window)
    {
        int width, height;
        glfwGetWindowSize(_window->get(), & width, & height);
    }
    
    void InputManager::init(nlohmann::json const& inputConfig)
    {
        _InputConfig = inputConfig;

        //@todo should not be here and take keyboard type (qwerty, azerty, etc).
         _KeyboardKeys = {
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

        glfwSetKeyCallback(_window->get(), []( GLFWwindow* window, int key, int scancode, int action, int mods) {
            InputManager* inputManager = Locator::getInputManager();
            inputManager->key(key, scancode, action, mods);
        });

        glfwSetCursorPosCallback(_window->get(), []( GLFWwindow* window, double xPos, double yPos) {
            InputManager* inputManager = Locator::getInputManager();
            inputManager->updateMousePos(xPos, yPos);
        });

        glfwSetMouseButtonCallback(_window->get(), []( GLFWwindow* window, int button, int action, int mods) {
            InputManager* inputManager = Locator::getInputManager();
            inputManager->mouseButton(button, action, mods);
        });
    }

    void InputManager::key(
        int const key,
         int const scancode,
        int const action,
         const int mods)
    {
        const auto config = _InputConfig[_InputConfig["current"]];

        switch (action)
        {
            case GLFW_PRESS:
            case GLFW_REPEAT:
            {
                if (key == _KeyboardKeys[config["unlockCamera"]]) {
                    if (!InputManager::_CanMoveCamera) {
                        glfwSetInputMode(_window->get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    } else {
                        glfwSetInputMode(_window->get(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                    }
                    InputManager::_CanMoveCamera = !InputManager::_CanMoveCamera;
                } else if (key == _KeyboardKeys[config["forward"]]) {
                    _camera->forward();
                } else if (key == _KeyboardKeys[config["backward"]]) {
                    _camera->backward();
                } else if (key == _KeyboardKeys[config["left"]]) {
                    _camera->left();
                } else if (key == _KeyboardKeys[config["right"]]) {
                    _camera->right();
                } else if (key == _KeyboardKeys[config["up"]]) {
                    _camera->up();
                } else if (key == _KeyboardKeys[config["down"]]) {
                    _camera->down();
                } else if (key == _KeyboardKeys[config["unlockFPS"]])
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

        double xpos, ypos;
        glfwGetCursorPos(_window->get(), &xpos, &ypos);
      }
    }

    void InputManager::saveLastMousePos(double const xPos, double const yPos)
    {
      _LastX = xPos;
      _LastY = yPos;

      InputManager::_FirtMouseMove = true;
    }

    void InputManager::updateMousePos(double const x, double const y)
    {
        double xPos = x;
        double yPos = y;

        if (!InputManager::_CanMoveCamera) return;

        if (InputManager::_FirtMouseMove) {
            _LastX = xPos;
            _LastY = yPos;
            InputManager::_FirtMouseMove = false;
        }

        double xoffset = xPos - _LastX;
        double yoffset = _LastY - yPos;

        _LastX = xPos;
        _LastY = yPos;
    
        const double sensitivity = 0.5;

        xoffset *= sensitivity;
        yoffset *= sensitivity;
    
        _camera->updatePos(xoffset, yoffset);
    }
}
