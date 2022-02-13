#pragma once
#include "InputManager.h"

namespace Rbk
{
	InputManager::InputManager(Window* window, Camera* camera) : m_Window(window), m_Camera(camera)
	{
		int width, height;
		glfwGetWindowSize(m_Window->Get(), &width, &height);
		m_LastX = 800;
		m_LastY = 600;
	}
	
	void InputManager::Init()
	{

		glfwSetWindowUserPointer(m_Window->Get(), this);

		glfwSetKeyCallback(m_Window->Get(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			InputManager inputManager = *(InputManager*)glfwGetWindowUserPointer(window);
			inputManager.Key(key, scancode, action, mods);
		});

		glfwSetCursorPosCallback(m_Window->Get(), [](GLFWwindow* window, double xPos, double yPos) {
			InputManager inputManager = *(InputManager*)glfwGetWindowUserPointer(window);
			inputManager.Mouse(xPos, yPos);
		});
	}

	void InputManager::Key(int key, int scancode, int action, int mods)
	{
		Rbk::Log::GetLogger()->debug("Event action {} key {} scancode {} mods {}", action, key, scancode, mods);

		switch (action)
		{
			case GLFW_REPEAT:
			case GLFW_PRESS:
			{
				if (glfwGetKey(m_Window->Get(), GLFW_KEY_W) == GLFW_PRESS) {
					m_Camera->Up();
				}
				if (glfwGetKey(m_Window->Get(), GLFW_KEY_S) == GLFW_PRESS) {
					m_Camera->Down();					
				}
				if (glfwGetKey(m_Window->Get(), GLFW_KEY_A) == GLFW_PRESS) {
					m_Camera->Left();
				}
				if (glfwGetKey(m_Window->Get(), GLFW_KEY_D) == GLFW_PRESS) {
					m_Camera->Right();
				}

				break;
			}
			case GLFW_RELEASE:
			{

				break;
			}
		}
	}

	void InputManager::Mouse(double x, double y)
	{
		float xPos = static_cast<float>(x);
		float yPos = static_cast<float>(y);

		float xoffset = xPos - m_LastX;
		float yoffset = m_LastY - yPos;
		m_LastX = xPos;
		m_LastY = yPos;
	
		const float sensitivity = 0.001f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;
	
		m_Camera->UpdateYP(xoffset, yoffset);
	}
}