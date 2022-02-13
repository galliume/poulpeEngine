#pragma once
#include "KeyManager.h"

namespace Rbk
{
	KeyManager::KeyManager(Window* window, Camera* camera) : m_Window(window), m_Camera(camera)
	{

	}
	
	void KeyManager::Init()
	{
		glfwSetWindowUserPointer(m_Window->Get(), this);

		glfwSetKeyCallback(m_Window->Get(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			KeyManager keyManager = *(KeyManager*)glfwGetWindowUserPointer(window);
			keyManager.Event(key, scancode, action, mods);
		});
	}

	void KeyManager::Event(int key, int scancode, int action, int mods)
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
}