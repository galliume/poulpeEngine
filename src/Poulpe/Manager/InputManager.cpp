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
        m_KeyboardKeys.insert({ "left_ctrl", GLFW_KEY_LEFT_CONTROL });
        m_KeyboardKeys.insert({ "a", GLFW_KEY_A });
        m_KeyboardKeys.insert({ "b", GLFW_KEY_B });
        m_KeyboardKeys.insert({ "c", GLFW_KEY_C });
        m_KeyboardKeys.insert({ "d", GLFW_KEY_D });
        m_KeyboardKeys.insert({ "e", GLFW_KEY_E });
        m_KeyboardKeys.insert({ "f", GLFW_KEY_F });
        m_KeyboardKeys.insert({ "g", GLFW_KEY_G });
        m_KeyboardKeys.insert({ "g", GLFW_KEY_H });
        m_KeyboardKeys.insert({ "i", GLFW_KEY_I });
        m_KeyboardKeys.insert({ "j", GLFW_KEY_J });
        m_KeyboardKeys.insert({ "k", GLFW_KEY_K });
        m_KeyboardKeys.insert({ "l", GLFW_KEY_L });
        m_KeyboardKeys.insert({ "m", GLFW_KEY_M });
        m_KeyboardKeys.insert({ "n", GLFW_KEY_N });
        m_KeyboardKeys.insert({ "o", GLFW_KEY_O });
        m_KeyboardKeys.insert({ "p", GLFW_KEY_P });
        m_KeyboardKeys.insert({ "q", GLFW_KEY_Q });
        m_KeyboardKeys.insert({ "r", GLFW_KEY_R });
        m_KeyboardKeys.insert({ "s", GLFW_KEY_S });
        m_KeyboardKeys.insert({ "t", GLFW_KEY_T });
        m_KeyboardKeys.insert({ "u", GLFW_KEY_U });
        m_KeyboardKeys.insert({ "v", GLFW_KEY_V });
        m_KeyboardKeys.insert({ "w", GLFW_KEY_W });
        m_KeyboardKeys.insert({ "x", GLFW_KEY_X });
        m_KeyboardKeys.insert({ "y", GLFW_KEY_Y });
        m_KeyboardKeys.insert({ "z", GLFW_KEY_Z });

        glfwSetWindowUserPointer(m_Window->Get(), this);

        glfwSetKeyCallback(m_Window->Get(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            InputManager* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
            inputManager->Key(key, scancode, action, mods);
        });

        glfwSetCursorPosCallback(m_Window->Get(), [](GLFWwindow* window, double xPos, double yPos) {
            InputManager* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
            inputManager->Mouse(xPos, yPos);
        });

        glfwSetMouseButtonCallback(m_Window->Get(), [](GLFWwindow* window, int button, int action, int mods) {
            InputManager* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
            inputManager->MouseButton(button, action, mods);
        });
    }

    void InputManager::Key(int key, int scancode, int action, int mods)
    {
        auto config = m_InputConfig[m_InputConfig["current"]];

        switch (action)
        {
            case GLFW_PRESS:
                if (glfwGetKey(m_Window->Get(), m_KeyboardKeys[config["unlockCamera"]])) {
                    PLP_DEBUG("ctrl clicked!");
                    if (!InputManager::m_CanMoveCamera) {
                        glfwSetInputMode(m_Window->Get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    } else {
                        glfwSetInputMode(m_Window->Get(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                    }
                    InputManager::m_CanMoveCamera = !InputManager::m_CanMoveCamera;
                }
            case GLFW_REPEAT:
            {
                if (glfwGetKey(m_Window->Get(), m_KeyboardKeys[config["forward"]]) ) {
                    m_Camera->Forward();
                }
                if (glfwGetKey(m_Window->Get(), m_KeyboardKeys[config["backward"]])) {
                    m_Camera->Backward();
                }
                if (glfwGetKey(m_Window->Get(), m_KeyboardKeys[config["left"]])) {
                    m_Camera->Left();
                }
                if (glfwGetKey(m_Window->Get(), m_KeyboardKeys[config["right"]])) {
                    m_Camera->Right();
                }
                if (glfwGetKey(m_Window->Get(), m_KeyboardKeys[config["up"]])) {
                    m_Camera->Up();
                }
                if (glfwGetKey(m_Window->Get(), m_KeyboardKeys[config["down"]])) {
                    m_Camera->Down();
                }
                break;
            }
            case GLFW_RELEASE:
            {
                
            }
        }
    }

    void InputManager::Mouse(double x, double y)
    {
        PLP_DEBUG("clicked !");
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

            m_Adapter->SetRayPick(x, y, z, width, height);
        }
    }
}
