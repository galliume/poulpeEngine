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
			//vImGuiInfo = rendererAdapter.get()->GetVImGuiInfo();		
		}
	}

	void Application::Run()
	{
		//todo move to Window
		glfwSetWindowUserPointer(window->Get(), renderManager->Adp());

		double lastTime = glfwGetTime();
		
		ImGui::CreateContext();

		vImGuiInfo = rendererAdapter.get()->GetVImGuiInfo();
		const char* glsl_version = "#version 150";
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		io.ConfigDockingWithShift = false;
		io.ConfigViewportsNoAutoMerge = true;
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;

		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);

		ImGui::CreateContext();
		ImGui_ImplGlfw_InitForVulkan(window->Get(), true);
		ImGui_ImplVulkan_Init(&vImGuiInfo.info, rendererAdapter.get()->RdrPass());

		rendererAdapter->ImmediateSubmit([&](VkCommandBuffer cmd) {
			ImGui_ImplVulkan_CreateFontsTexture(cmd);
		});

		ImGui_ImplVulkan_DestroyFontUploadObjects();
		
		while (!glfwWindowShouldClose(window->Get())) {


			double currentTime = glfwGetTime();
			double timeStep = currentTime - lastTime;

			glfwPollEvents();
			
			ImGui_ImplGlfw_NewFrame();
			ImGui_ImplVulkan_NewFrame();
			ImGui::NewFrame();
			
			vulkanLayer->DisplayFpsCounter(timeStep);
			vulkanLayer->DisplayAPI(rendererAdapter->Rdr()->GetDeviceProperties());
			vulkanLayer->DisplayLogs();

			ImGui::Render();

			ImDrawData* main_draw_data = ImGui::GetDrawData();
			ImGuiIO& io = ImGui::GetIO();

			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}


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