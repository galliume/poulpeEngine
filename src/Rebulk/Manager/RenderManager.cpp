#include "RenderManager.h"

namespace Rbk
{
	RenderManager* RenderManager::s_Instance = nullptr;

	RenderManager::RenderManager(GLFWwindow* window, IRendererAdapter* renderer, TextureManager* textureManager, MeshManager* meshManager)
	{
		m_Window = window;
		m_Renderer = renderer;
		m_TextureManager = textureManager;
		m_MeshManager = meshManager;

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
		m_Renderer->Init();
		m_Renderer->AddTextureManager(m_TextureManager);
		m_Renderer->AddMeshManager(m_MeshManager);

		auto vertShaderCode = ReadFile("shaders/spv/vert.spv");
		auto fragShaderCode = ReadFile("shaders/spv/frag.spv");

		AddShader("main", vertShaderCode, fragShaderCode);
		int width, height;
		glfwGetWindowSize(m_Window, &width, &height);

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
		//AddMesh("mesh/viking/viking_room.obj", "viking_room", pos1);
	}

	void RenderManager::AddCamera(Camera* camera)
	{
		m_Renderer->AddCamera(camera);
	}

	void RenderManager::AddShader(std::string name, std::vector<char> vertShaderCode, std::vector<char> fragShaderCode)
	{
		//@todo add ShaderManager
		m_Renderer->AddShader(name, vertShaderCode, fragShaderCode);
	}

	void RenderManager::PrepareDraw()
	{
		m_Renderer->PrepareDraw();
	}

	void RenderManager::Draw()
	{
		m_Renderer->Draw();
	}
}