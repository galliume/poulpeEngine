#include "InputManager.hpp"


namespace Poulpe
{
    bool InputManager::m_CanMoveCamera = false;
    bool InputManager::m_FirtMouseMove = true;

    InputManager::InputManager(Window* const window) : m_Window(window)
    {
        int width, height;
        glfwGetWindowSize(m_Window->get(), & width, & height);
    }
    
    void InputManager::init(nlohmann::json const inputConfig)
    {
        m_InputConfig = inputConfig;

        //@todo should not be here and take keyboard type (qwerty, azerty, etc).
         m_KeyboardKeys = {
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

        glfwSetWindowUserPointer(m_Window->get(), this);

        glfwSetKeyCallback(m_Window->get(), []([[maybe_unused]] GLFWwindow* window, int key, int scancode, int action, int mods) {
            InputManager* inputManager = Locator::getInputManager();
            inputManager->key(key, scancode, action, mods);
        });

        glfwSetCursorPosCallback(m_Window->get(), []([[maybe_unused]] GLFWwindow* window, double xPos, double yPos) {
            InputManager* inputManager = Locator::getInputManager();
            inputManager->updateMousePos(xPos, yPos);
        });

        glfwSetMouseButtonCallback(m_Window->get(), []([[maybe_unused]] GLFWwindow* window, int button, int action, int mods) {
            InputManager* inputManager = Locator::getInputManager();
            inputManager->mouseButton(button, action, mods);
        });
    }

    void InputManager::key(
        int const key,
        [[maybe_unused]] int const scancode,
        int const action,
        [[maybe_unused]] const int mods)
    {
        const auto config = m_InputConfig[m_InputConfig["current"]];

        switch (action)
        {
            case GLFW_PRESS:
            case GLFW_REPEAT:
            {
                if (key == m_KeyboardKeys[config["unlockCamera"]]) {
                    if (!InputManager::m_CanMoveCamera) {
                        glfwSetInputMode(m_Window->get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    } else {
                        glfwSetInputMode(m_Window->get(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                    }
                    InputManager::m_CanMoveCamera = !InputManager::m_CanMoveCamera;
                } else if (key == m_KeyboardKeys[config["forward"]]) {
                    m_Camera->forward();
                } else if (key == m_KeyboardKeys[config["backward"]]) {
                    m_Camera->backward();
                } else if (key == m_KeyboardKeys[config["left"]]) {
                    m_Camera->left();
                } else if (key == m_KeyboardKeys[config["right"]]) {
                    m_Camera->right();
                } else if (key == m_KeyboardKeys[config["up"]]) {
                    m_Camera->up();
                } else if (key == m_KeyboardKeys[config["down"]]) {
                    m_Camera->down();
                } else if (key == m_KeyboardKeys[config["unlockFPS"]])
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
        [[maybe_unused]] int const action,
        [[maybe_unused]] int const mods)
    {
      if (GLFW_MOUSE_BUTTON_LEFT == button) {
        int width, height;
        glfwGetWindowSize(m_Window->get(), &width, &height);

        double xpos, ypos;
        glfwGetCursorPos(m_Window->get(), &xpos, &ypos);
      }
    }

    void InputManager::saveLastMousePos(double const xPos, double const yPos)
    {
      m_LastX = xPos;
      m_LastY = yPos;

      InputManager::m_FirtMouseMove = true;
    }

    void InputManager::updateMousePos(double const x, double const y)
    {
        double xPos = x;
        double yPos = y;

        if (!InputManager::m_CanMoveCamera) return;

        if (InputManager::m_FirtMouseMove) {
            m_LastX = xPos;
            m_LastY = yPos;
            InputManager::m_FirtMouseMove = false;
        }

        double xoffset = xPos - m_LastX;
        double yoffset = m_LastY - yPos;

        m_LastX = xPos;
        m_LastY = yPos;
    
        const double sensitivity = 0.5;

        xoffset *= sensitivity;
        yoffset *= sensitivity;
    
        m_Camera->updatePos(xoffset, yoffset);
    }
}
