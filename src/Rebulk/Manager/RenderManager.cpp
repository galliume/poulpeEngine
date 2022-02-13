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

		UniformBufferObject ubo{};
		ubo.model = glm::mat4(1.0f);
		ubo.model *= glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
		ubo.view = glm::mat4(1.0f);

		UniformBufferObject ubo2{};
		ubo2.model = glm::mat4(1.0f);
		ubo2.model = glm::scale(ubo2.model, glm::vec3(0.3f, 0.3f, 0.3f));
		ubo2.view = glm::mat4(1.0f);

		UniformBufferObject ubo3{};
		ubo3.model = glm::mat4(1.0f);
		ubo3.model = glm::scale(ubo3.model, glm::vec3(0.2f, 0.2f, 0.2f));
		ubo3.model *= glm::translate(glm::mat4(1.0f), glm::vec3(-6.0f, 0.0f, 0.0f));
		ubo3.view = glm::mat4(1.0f);	

		AddTexture("viking_room", "mesh/viking/viking_room.png");
		AddTexture("diffuse_backpack",  "mesh/backpack/diffuse.png");
		AddTexture("diffuse_moon", "mesh/moon/diffuse.jpg");
		AddTexture("minecraft_grass", "mesh/minecraft/Grass_Block_TEX.png");

		UniformBufferObject minecraftGrass{};
		minecraftGrass.model = glm::mat4(1.0f);
		minecraftGrass.model *= glm::scale(glm::mat4(1.0f), glm::vec3(0.4f, 0.4f, 0.4f));		
		//minecraftGrass.model *= glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		minecraftGrass.view = glm::mat4(1.0f);

		//AddMesh("mesh/minecraft/Grass_Block.obj", "minecraft_grass", minecraftGrass);
		AddMesh("mesh/moon/moon.obj", "diffuse_moon", ubo2);
		//AddMesh("mesh/backpack/backpack.obj", "diffuse_backpack", ubo3, false);
		AddMesh("mesh/viking/viking_room.obj", "viking_room", ubo);
		//AddMesh("mesh/backpack/backpack.obj", "diffuse_backpack", ubo3, false);
		AddMesh("mesh/backpack/backpack.obj", "diffuse_backpack", ubo3, false);
	}

	void RenderManager::AddCamera(Camera* camera)
	{
		m_Renderer->AddCamera(camera);
	}

	void RenderManager::AddMesh(const char* path, const char* textureName, UniformBufferObject ubo, bool shouldInverseTextureY)
	{
		Mesh mesh;
		Rbk::TinyObjLoader::LoadMesh(mesh, path, shouldInverseTextureY);

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