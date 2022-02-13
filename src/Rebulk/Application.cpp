#include "Application.h"

namespace Rbk
{
	std::shared_ptr<Rbk::Window>window = nullptr;
	std::shared_ptr<Rbk::RenderManager>renderManager = nullptr;
	std::shared_ptr<Rbk::Camera>camera= nullptr;
	std::shared_ptr<Rbk::KeyManager>keyManager = nullptr;
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
			window.get()->Init();
		}

		if (camera == nullptr) {
			int width, height;
			glfwGetWindowSize(window->Get(), &width, &height);
			camera = std::make_shared<Rbk::Camera>(Rbk::Camera());
			camera->Init(width, height);
		}

		if (keyManager == nullptr) {
			keyManager = std::make_shared<Rbk::KeyManager>(Rbk::KeyManager(window.get(), camera.get()));
			keyManager->Init();
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
			renderManager->AddCamera(camera.get());
		}
	}

	void Application::Run()
	{
		double lastTime = glfwGetTime();
		
		VImGuiInfo imguiInfo = rendererAdapter->GetVImGuiInfo();
		imguiInfo.rdrPass = rendererAdapter.get()->CreateImGuiRenderPass();

		Rbk::Im::Init(window->Get(), imguiInfo.info, *imguiInfo.rdrPass);

		rendererAdapter->ImmediateSubmit([&](VkCommandBuffer cmd) {
			ImGui_ImplVulkan_CreateFontsTexture(cmd);
		});

		ImGui_ImplVulkan_DestroyFontUploadObjects();
		bool wireFrameModeOn = false;

		vulkanLayer->AddWindow(window.get());

		while (!glfwWindowShouldClose(window->Get())) {

			double currentTime = glfwGetTime();
			double timeStep = currentTime - lastTime;

			camera->UpdateSpeed(timeStep);

			glfwPollEvents();

			renderManager->PrepareDraw();
			renderManager->Draw();

			Rbk::Im::NewFrame();
			vulkanLayer->AddRenderAdapter(rendererAdapter.get());
			vulkanLayer->Render(timeStep, rendererAdapter->Rdr()->GetDeviceProperties());
			vulkanLayer->DisplayOptions();
			Rbk::Im::Render();

			rendererAdapter->Rdr()->BeginCommandBuffer(imguiInfo.cmdBuffer);
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), imguiInfo.cmdBuffer);
			rendererAdapter->Rdr()->EndCommandBuffer(imguiInfo.cmdBuffer);

			glfwSwapBuffers(window->Get());
			lastTime = currentTime;
		}

		Rbk::Im::Destroy();

		renderManager->Adp()->Destroy();

		glfwDestroyWindow(window->Get());
		glfwTerminate();
	}
}