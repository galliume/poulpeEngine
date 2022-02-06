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

		if (vImGui == nullptr)
		{		
			vImGuiInfo = rendererAdapter.get()->GetVImGuiInfo();		
		}
	}

	void Application::Run()
	{
		//todo move to Window
		glfwSetWindowUserPointer(window->Get(), renderManager->Adp());

		double lastTime = glfwGetTime();
		bool show_demo_window = true;
		bool show_another_window = true;

		Rbk::Im::Init(window->Get(), vImGuiInfo.info, rendererAdapter.get()->RdrPass());

		rendererAdapter.get()->Rdr()->BeginCommandBuffer(vImGuiInfo.cmdBuffer);
		Rbk::Im::CreateFontsTexture(vImGuiInfo.cmdBuffer);
		rendererAdapter.get()->Rdr()->EndCommandBuffer(vImGuiInfo.cmdBuffer);

		while (!glfwWindowShouldClose(window->Get())) {


			double currentTime = glfwGetTime();
			double timeStep = currentTime - lastTime;

			glfwPollEvents();

			
			Rbk::Im::NewFrame();
			ImGui::ShowDemoWindow(&show_demo_window);

			rendererAdapter.get()->Rdr()->BeginCommandBuffer(vImGuiInfo.cmdBuffer);
			//layerManager->InitLayers();

			ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();

			Rbk::Im::Render(window->Get(), vImGuiInfo.cmdBuffer, vImGuiInfo.pipeline);
			
			//renderManager->Draw();

			glfwSwapBuffers(window->Get());
			lastTime = currentTime;
		}

		Rbk::Im::Destroy();

		renderManager->Adp()->Destroy();

		glfwDestroyWindow(window->Get());
		glfwTerminate();
	}
}