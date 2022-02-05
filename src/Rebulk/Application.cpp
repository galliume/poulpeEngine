#include "Application.h"
#include "Renderer/Adapter/VulkanAdapter.h"

namespace Rbk
{
	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		Rbk::Log::Init();

		if (window == nullptr) {
			window = std::make_shared<Rbk::Window>(Rbk::Window());
		}

		if (renderManager == nullptr) {

			rendererAdapter = std::make_shared<Rbk::VulkanAdapter>(Rbk::VulkanAdapter(window->Get()));
			rendererAdapter->Init();
			renderManager = std::make_shared<Rbk::RenderManager>(Rbk::RenderManager(window->Get(), rendererAdapter.get()));
		}

		if (s_Instance == nullptr) {
			s_Instance = this;
		}
	}

	Application::~Application()
	{
		std::cout << "Application deleted" << std::endl;
	}

	void Application::Run()
	{
		renderManager->Init();

		//todo move to Window
		glfwSetWindowUserPointer(window->Get(), renderManager->Rdr());

		double lastTime = glfwGetTime();
		bool show_demo_window = true;

		while (!glfwWindowShouldClose(window->Get())) {

			double currentTime = glfwGetTime();
			double timeStep = currentTime - lastTime;

			glfwPollEvents();

			renderManager->Rdr()->Draw();

			glfwSwapBuffers(window->Get());
			lastTime = currentTime;
		}

		renderManager->Rdr()->Destroy();

		glfwDestroyWindow(window->Get());
		glfwTerminate();
	}
}