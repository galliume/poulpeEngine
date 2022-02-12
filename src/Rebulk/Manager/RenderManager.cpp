#include "RenderManager.h"

namespace Rbk
{
	RenderManager* RenderManager::s_Instance = nullptr;

	RenderManager::RenderManager(GLFWwindow* window, IRendererAdapter* renderer)
	{
		m_Renderer = renderer;
		m_Window = window;

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

		auto vertShaderCode = ReadFile("shaders/spv/vert.spv");
		auto fragShaderCode = ReadFile("shaders/spv/frag.spv");

		AddShader("main", vertShaderCode, fragShaderCode);

		int width, height;
		glfwGetWindowSize(m_Window, &width, &height);

		UniformBufferObject ubo{};
		ubo.model = glm::mat4(1.0f);
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 10.0f);
		ubo.proj[1][1] *= -1;

		UniformBufferObject ubo2{};
		ubo2.model = glm::mat4(1.0f);
		ubo2.model = glm::scale(ubo2.model, glm::vec3(0.3f, 0.3f, 0.3f));
		ubo2.view = glm::lookAt(glm::vec3(0.0f, -0.5f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo2.proj = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 10.0f);
		ubo2.proj[1][1] *= -1;

		UniformBufferObject ubo3{};
		ubo3.model = glm::mat4(1.0f);
		ubo3.model = glm::scale(ubo3.model, glm::vec3(0.2f, 0.2f, 0.2f));
		ubo3.model *= glm::translate(glm::mat4(1.0f), glm::vec3(-6.0f, 0.0f, 0.0f));
		ubo3.view = glm::lookAt(glm::vec3(0.0f, -0.5f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo3.proj = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 10.0f);
		ubo3.proj[1][1] *= -1;


		AddTexture("diffuse_backpack",  "mesh/backpack/diffuse.png");
		AddTexture("diffuse_moon", "mesh/moon/diffuse.jpg");
		AddTexture("minecraft_grass", "mesh/minecraft/Grass_Block_TEX.png");
		AddTexture("viking_room", "mesh/viking/viking_room.png");

		UniformBufferObject minecraftGrass{};
		minecraftGrass.model = glm::mat4(1.0f);
		minecraftGrass.model *= glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
		
		minecraftGrass.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 1.0f));
		minecraftGrass.proj = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 10.0f);
		minecraftGrass.proj[1][1] *= -1;

		//AddMesh("mesh/viking/viking_room.obj", "viking_room", ubo);
		AddMesh("mesh/backpack/backpack.obj", "diffuse_backpack", ubo2);
		//AddMesh("mesh/minecraft/Grass_Block.obj", "minecraft_grass", minecraftGrass);
		AddMesh("mesh/backpack/backpack.obj", "diffuse_backpack", ubo3);
		//AddMesh("mesh/kitty/kitty.obj", "diffuse_moon", ubo2);
	}

	void RenderManager::AddMesh(const char* path, const char* textureName, UniformBufferObject ubo, bool shouldInverseTextureY)
	{
		Mesh mesh;
		Rbk::TinyObjLoader::LoadMesh(mesh, path);

		//@todo Add MeshManager
		m_Renderer->AddMesh(mesh, textureName, ubo);
	}

	void RenderManager::AddTexture(const char* name, const char* path)
	{
		//@todo add TextureManager
		m_Renderer->AddTexture(name, path);
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