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
			rendererAdapter = std::make_shared<Rbk::VulkanAdapter>(Rbk::VulkanAdapter(window->Get()));
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

		//AddMesh("mesh/minecraft/Grass_Block.obj", "minecraft_grass", minecraftGrass);
		//AddMesh("mesh/moon/moon.obj", "diffuse_moon", ubo2);
		//AddMesh("mesh/backpack/backpack.obj", "diffuse_backpack", ubo3, false);
		//AddMesh("mesh/viking/viking_room.obj", "viking_room", ubo);
		//AddMesh("mesh/backpack/backpack.obj", "diffuse_backpack", ubo3, false);

		glm::vec3 pos1 = glm::vec3(0.1f, 0.1f, 0.1f);
		glm::vec3 pos2 = glm::vec3(-0.5f, -0.5f, -0.5f);
		glm::vec3 pos3 = glm::vec3(0.1f, -0.8f, 0.0f);

		//AddTexture("viking_room", "mesh/viking/viking_room.png");
		//AddTexture("diffuse_backpack", "mesh/backpack/diffuse.png");
		//AddTexture("diffuse_moon", "mesh/moon/diffuse.jpg");

		//AddMesh("mesh/backpack/backpack.obj", "diffuse_backpack", pos2, false);

		shaderManager->AddShader("main", "shaders/spv/vert.spv", "shaders/spv/frag.spv");

		//textureManager->AddTexture("viking_room", "mesh/viking/viking_room.png");
		textureManager->AddTexture("minecraft_grass", "mesh/minecraft/Grass_Block_TEX.png");

		//meshManager->AddMesh("room", "mesh/viking/viking_room.obj", "viking_room", pos1);
		meshManager->AddMesh("cube", "mesh/minecraft/Grass_Block.obj", "minecraft_grass", pos3);

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