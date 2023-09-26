#include "InputManager.hpp"

namespace Poulpe
{
    bool InputManager::m_CanMoveCamera = false;
    bool InputManager::m_FirtMouseMove = true;

    InputManager::InputManager(std::shared_ptr<Window> window) : m_Window(window)
    {
        int width, height;
        glfwGetWindowSize(m_Window->Get(), &width, &height);
        m_LastX = 800;
        m_LastY = 600;
        InputManager::m_FirtMouseMove = true;
    }
    
    void InputManager::Init(nlohmann::json inputConfig)
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
            { "z", GLFW_KEY_Z }
        };

        glfwSetWindowUserPointer(m_Window->Get(), this);

        glfwSetKeyCallback(m_Window->Get(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            InputManager* inputManager = Poulpe::Locator::getInputManager();
            inputManager->Key(key, scancode, action, mods);
        });

        glfwSetCursorPosCallback(m_Window->Get(), [](GLFWwindow* window, double xPos, double yPos) {
            InputManager* inputManager = Poulpe::Locator::getInputManager();
            inputManager->Mouse(xPos, yPos);
        });

        glfwSetMouseButtonCallback(m_Window->Get(), [](GLFWwindow* window, int button, int action, int mods) {
            InputManager* inputManager = Poulpe::Locator::getInputManager();
            inputManager->MouseButton(button, action, mods);
        });
    }

    void InputManager::Key(int key, int scancode, int action, int mods)
    {
        const auto config = m_InputConfig[m_InputConfig["current"]];

        switch (action)
        {
            case GLFW_PRESS:
            case GLFW_REPEAT:
            {
                if (key == m_KeyboardKeys[config["unlockCamera"]]) {
                    if (!InputManager::m_CanMoveCamera) {
                        glfwSetInputMode(m_Window->Get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    } else {
                        glfwSetInputMode(m_Window->Get(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                    }
                    InputManager::m_CanMoveCamera = !InputManager::m_CanMoveCamera;
                } else if (key == m_KeyboardKeys[config["forward"]]) {
                    m_Camera->Forward();
                } else if (key == m_KeyboardKeys[config["backward"]]) {
                    m_Camera->Backward();
                } else if (key == m_KeyboardKeys[config["left"]]) {
                    m_Camera->Left();
                } else if (key == m_KeyboardKeys[config["right"]]) {
                    m_Camera->Right();
                } else if (key == m_KeyboardKeys[config["up"]]) {
                    m_Camera->Up();
                } else if (key == m_KeyboardKeys[config["down"]]) {
                    m_Camera->Down();
                }
                break;
            }
            case GLFW_RELEASE:
            {
                //nothing, yet.
            }
        }
    }

    void InputManager::Mouse(double x, double y)
    {
        float xPos = static_cast<float>(x);
        float yPos = static_cast<float>(y);

        if (!InputManager::m_CanMoveCamera) return;

        if (InputManager::m_FirtMouseMove) {
            m_LastX = xPos;
            m_LastY = yPos;
            InputManager::m_FirtMouseMove = false;
        }

        float xoffset = xPos - m_LastX;
        float yoffset = m_LastY - yPos;
        m_LastX = xPos;
        m_LastY = yPos;
    
        const float sensitivity = 0.25f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;
    
        m_Camera->UpdateYP(xoffset, yoffset);
    }

    void InputManager::MouseButton(int button, int action, int mods)
    {
        if (GLFW_MOUSE_BUTTON_LEFT == button) {
            int width, height;
            glfwGetWindowSize(m_Window->Get(), &width, &height);

            double xpos, ypos;
            glfwGetCursorPos(m_Window->Get(), &xpos, &ypos);

            float x = (2.0f * xpos) / width - 1.0f;
            float y = 1.0f - (2.0f * ypos) / height;
            float z = 1.0f;

            //m_Adapter->SetRayPick(x, y, z, width, height);
        }
    }
}
