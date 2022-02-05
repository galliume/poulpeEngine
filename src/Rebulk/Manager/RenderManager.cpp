#include "RenderManager.h"

namespace Rbk
{
	RenderManager* RenderManager::s_Instance = nullptr;

	RenderManager::RenderManager(GLFWwindow* window, IRendererAdapter* renderer)
	{
		m_Renderer = renderer;

		if (s_Instance == nullptr) {
			s_Instance = this;
		}
	}

	RenderManager::~RenderManager()
	{
		std::cout << "RenderManager deleted" << std::endl;
	}

	void RenderManager::Init()
	{

	}
}