#include "Application.h"
#include "Renderer/Adapter/VulkanAdapter.h"

#include "GUI/LayerManager.h"
#include "GUI/Layer/VulkanLayer.h"
#include "GUI/ImGui/Im.h"

namespace Rbk
{
	std::shared_ptr<Rbk::Window>window = nullptr;
	std::shared_ptr<Rbk::RenderManager>renderManager = nullptr;
	std::shared_ptr<Rbk::VulkanAdapter>rendererAdapter = nullptr;

	std::shared_ptr<Rbk::LayerManager>layerManager = nullptr;
	std::shared_ptr<Rbk::VulkanLayer>vulkanLayer = nullptr;
	std::shared_ptr<Rbk::Im>vImGui = nullptr;

	Application* Application::s_Instance = nullptr;

	VImGuiInfo vImGuiInfo;

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

		if (layerManager == nullptr) {
			vulkanLayer = std::make_shared<Rbk::VulkanLayer>(Rbk::VulkanLayer());
			layerManager = std::make_shared<Rbk::LayerManager>(Rbk::LayerManager());

			layerManager->Add(vulkanLayer.get());
		}

		if (renderManager == nullptr) {
			rendererAdapter = std::make_shared<Rbk::VulkanAdapter>(Rbk::VulkanAdapter(window->Get()));
			renderManager = std::make_shared<Rbk::RenderManager>(Rbk::RenderManager(window->Get(), rendererAdapter.get()));
			renderManager->Init();
		}
	}

	void Application::Run()
	{
		//todo move to Window
		glfwSetWindowUserPointer(window->Get(), renderManager->Adp());

		double lastTime = glfwGetTime();
		
		Rbk::Im::Init(window->Get(), rendererAdapter->GetVImGuiInfo().info, rendererAdapter.get()->RdrPass());

		rendererAdapter->ImmediateSubmit([&](VkCommandBuffer cmd) {
			ImGui_ImplVulkan_CreateFontsTexture(cmd);
		});

		ImGui_ImplVulkan_DestroyFontUploadObjects();
		bool wireFrameModeOn = false;

		while (!glfwWindowShouldClose(window->Get())) {

			double currentTime = glfwGetTime();
			double timeStep = currentTime - lastTime;

			glfwPollEvents();

			Rbk::Im::NewFrame();
			vulkanLayer->AddRenderAdapter(rendererAdapter.get());
			vulkanLayer->Render(timeStep, rendererAdapter->Rdr()->GetDeviceProperties());
			vulkanLayer->DisplayOptions();

			ImGui::ShowDemoWindow();
			Rbk::Im::Render();

			renderManager->PrepareDraw();
			renderManager->Draw();

			glfwSwapBuffers(window->Get());
			lastTime = currentTime;
		}

		Rbk::Im::Destroy();

		renderManager->Adp()->Destroy();

		glfwDestroyWindow(window->Get());
		glfwTerminate();
	}
}