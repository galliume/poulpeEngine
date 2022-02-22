#include "Application.h"

namespace Rbk
{
	std::shared_ptr<Rbk::Window>window = nullptr;
	std::shared_ptr<Rbk::RenderManager>renderManager = nullptr;
	std::shared_ptr<Rbk::Camera>camera= nullptr;
	std::shared_ptr<Rbk::InputManager>inputManager = nullptr;
	std::shared_ptr<Rbk::VulkanAdapter>rendererAdapter = nullptr;
	std::shared_ptr<Rbk::LayerManager>layerManager = nullptr;
	std::shared_ptr<Rbk::TextureManager>textureManager = nullptr;
	std::shared_ptr<Rbk::MeshManager>meshManager = nullptr;
	std::shared_ptr<Rbk::ShaderManager>shaderManager = nullptr;
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

		if (inputManager == nullptr) {
			inputManager = std::make_shared<Rbk::InputManager>(Rbk::InputManager(window.get(), camera.get()));
			inputManager->Init();
		}

		if (layerManager == nullptr) {
			vulkanLayer = std::make_shared<Rbk::VulkanLayer>(Rbk::VulkanLayer());
			layerManager = std::make_shared<Rbk::LayerManager>(Rbk::LayerManager());

			layerManager->Add(vulkanLayer.get());
		}

		if (rendererAdapter == nullptr) {
			rendererAdapter = std::make_shared<Rbk::VulkanAdapter>(Rbk::VulkanAdapter(window.get()));
		}

		if (textureManager == nullptr) {
			textureManager = std::make_shared<Rbk::TextureManager>(Rbk::TextureManager(rendererAdapter.get()->Rdr()));
		}

		if (meshManager == nullptr) {
			meshManager = std::make_shared<Rbk::MeshManager>(Rbk::MeshManager(rendererAdapter.get()->Rdr()));
		}

		if (shaderManager == nullptr) {
			shaderManager = std::make_shared<Rbk::ShaderManager>(Rbk::ShaderManager(rendererAdapter.get()->Rdr()));
		}

		if (renderManager == nullptr) {
			renderManager = std::make_shared<Rbk::RenderManager>(Rbk::RenderManager(
				window->Get(), rendererAdapter.get(), textureManager.get(), meshManager.get(), shaderManager.get()
			));
			renderManager->Init();
			renderManager->AddCamera(camera.get());
		}
	}

	void Application::Run()
	{		
		glm::vec3 pos1 = glm::vec3(0.01f, 0.01f, 0.01f);
		glm::vec3 pos2 = glm::vec3(-0.5f, -0.5f, -0.5f);
		glm::vec3 pos3 = glm::vec3(0.1f, -0.8f, 0.0f);
		glm::vec3 pos4 = glm::vec3(0.6f, -0.18f, 0.5f);
		glm::vec3 pos5 = glm::vec3(1.6f, -0.55f, 1.5f);		

		shaderManager->AddShader("main", "shaders/spv/vert.spv", "shaders/spv/frag.spv");

		textureManager->AddTexture("diffuse_moon", "mesh/moon/diffuse.jpg");
		textureManager->AddTexture("minecraft_grass", "mesh/minecraft/Grass_Block_TEX.png");
		textureManager->AddTexture("viking_room", "mesh/viking/viking_room.png");
		
		//for (int x = 0; x < 3; x++) {
		//	for (int y = 0; y < 3; y++) {
				//glm::vec3 posCube = glm::vec3(0.3f / x, -0.32f, 1.0f / y);
				//glm::vec3 posMoon = glm::vec3(0.3f / x, -0.15f, 1.0f / y);

				meshManager->AddWorldMesh("cube", "mesh/minecraft/Grass_Block.obj", "minecraft_grass", pos1);
				//meshManager->AddWorldMesh("moon", "mesh/moon/moon.obj", "diffuse_moon", posMoon);
		//	}
		//}

		//meshManager->AddWorldMesh("room", "mesh/viking/viking_room.obj", "viking_room", pos4);

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
		vulkanLayer->AddTextureManager(textureManager.get());
		vulkanLayer->AddMeshManager(meshManager.get());
		vulkanLayer->AddShaderManager(shaderManager.get());

		rendererAdapter->PrepareWorld();

		while (!glfwWindowShouldClose(window->Get())) {

			double currentTime = glfwGetTime();
			double timeStep = currentTime - lastTime;

			camera->UpdateSpeed(timeStep);

			glfwPollEvents();

			renderManager->PrepareDraw();
			renderManager->Draw();

			//@todo move to LayerManager
			Rbk::Im::NewFrame();
			vulkanLayer->AddRenderAdapter(rendererAdapter.get());
			vulkanLayer->Render(timeStep, rendererAdapter->Rdr()->GetDeviceProperties());
			vulkanLayer->DisplayOptions();
			Rbk::Im::Render();

			rendererAdapter->Rdr()->BeginCommandBuffer(imguiInfo.cmdBuffer);
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), imguiInfo.cmdBuffer);
			rendererAdapter->Rdr()->EndCommandBuffer(imguiInfo.cmdBuffer);
			//end @todo

			glfwSwapBuffers(window->Get());
			lastTime = currentTime;
		}

		Rbk::Im::Destroy();

		renderManager->Adp()->Destroy();

		glfwDestroyWindow(window->Get());
		glfwTerminate();
	}
}