#include "rebulkpch.h"
#include "InputManager.h"

namespace Rbk
{
    bool InputManager::m_CanMoveCamera = false;
    bool InputManager::m_FirtMouseMove = true;

    InputManager::InputManager(std::shared_ptr<Window> window, std::shared_ptr<Camera> camera) : m_Window(window), m_Camera(camera)
    {
        int width, height;
        //@todo ugly
        glfwGetWindowSize(m_Window.get()->Get(), &width, &height);
        m_LastX = 800;
        m_LastY = 600;
        InputManager::m_FirtMouseMove = true;
    }
    
    void InputManager::Init()
    {
        glfwSetWindowUserPointer(m_Window.get()->Get(), this);

        glfwSetKeyCallback(m_Window.get()->Get(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            InputManager inputManager = *(InputManager*)glfwGetWindowUserPointer(window);
            inputManager.Key(key, scancode, action, mods);
            });

        glfwSetCursorPosCallback(m_Window.get()->Get(), [](GLFWwindow* window, double xPos, double yPos) {
            InputManager inputManager = *(InputManager*)glfwGetWindowUserPointer(window);
            inputManager.Mouse(xPos, yPos);
            });

        glfwSetMouseButtonCallback(m_Window.get()->Get(), [](GLFWwindow* window, int button, int action, int mods) {
            InputManager inputManager = *(InputManager*)glfwGetWindowUserPointer(window);
            inputManager.MouseButton(button, action, mods);
            });
    }

    void InputManager::Key(int key, int scancode, int action, int mods)
    {
        //Rbk::Log::GetLogger()->debug("Event action {} key {} scancode {} mods {}", action, key, scancode, mods);

        //Rbk::Log::GetLogger()->debug("Camera pos x : {} y : {} z : {}",
        //    m_Camera->GetPos().x,
        //    m_Camera->GetPos().y,
        //    m_Camera->GetPos().z
        //);

        switch (action)
        {
            case GLFW_PRESS:
                if (glfwGetKey(m_Window.get()->Get(), GLFW_KEY_LEFT_CONTROL)) {

                    if (!InputManager::m_CanMoveCamera) {
                        glfwSetInputMode(m_Window.get()->Get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    } else {
                        glfwSetInputMode(m_Window.get()->Get(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                    }
                    InputManager::m_CanMoveCamera = !InputManager::m_CanMoveCamera;
                }
            case GLFW_REPEAT:
            {
                if (glfwGetKey(m_Window.get()->Get(), GLFW_KEY_C)) {
                    m_Camera->Recenter();
                }
                if (glfwGetKey(m_Window.get()->Get(), GLFW_KEY_W) ) {
                    m_Camera->Forward();
                }
                if (glfwGetKey(m_Window.get()->Get(), GLFW_KEY_S)) {
                    m_Camera->Backward();
                }
                if (glfwGetKey(m_Window.get()->Get(), GLFW_KEY_A)) {
                    m_Camera->Left();
                }
                if (glfwGetKey(m_Window.get()->Get(), GLFW_KEY_D)) {
                    m_Camera->Right();
                }
                if (glfwGetKey(m_Window.get()->Get(), GLFW_KEY_Q)) {
                    m_Camera->Up();
                }
                if (glfwGetKey(m_Window.get()->Get(), GLFW_KEY_E)) {
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
        if (!InputManager::m_CanMoveCamera) return;

        float xPos = static_cast<float>(x);
        float yPos = static_cast<float>(y);

        if (InputManager::m_FirtMouseMove) {
            m_LastX = xPos;
            m_LastY = yPos;
            InputManager::m_FirtMouseMove = false;
        }

        float xoffset = xPos - m_LastX;
        float yoffset = m_LastY - yPos;
        m_LastX = xPos;
        m_LastY = yPos;
    
        const float sensitivity = 0.001f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;
    
        m_Camera->UpdateYP(xoffset, yoffset);
    }

    void InputManager::MouseButton(int button, int action, int mods)
    {

    }
}
