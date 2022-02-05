#include "Application.h"
#include "Renderer/Adapter/VulkanAdapter.h"

namespace Rbk
{
	std::shared_ptr<Rbk::Window>window = nullptr;
	std::shared_ptr<Rbk::RenderManager>renderManager = nullptr;
	std::shared_ptr<Rbk::IRendererAdapter>rendererAdapter = nullptr;

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		if (s_Instance == nullptr) {
			s_Instance = this;
		}
	}

	Application::~Application()
	{
		std::cout << "Application deleted" << std::endl;
	}

	void Application::Init()
	{
		Rbk::Log::Init();

		if (window == nullptr) {
			window = std::make_shared<Rbk::Window>(Rbk::Window());
		}

		if (renderManager == nullptr) {
			rendererAdapter = std::make_shared<Rbk::VulkanAdapter>(Rbk::VulkanAdapter(window->Get()));
			renderManager = std::make_shared<Rbk::RenderManager>(Rbk::RenderManager(window->Get(), rendererAdapter.get()));
		}
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