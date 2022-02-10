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
		ubo.model = glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f);
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 10.0f);
		ubo.proj[1][1] *= -1;

		//AddMesh("mesh/moon/moon.obj", "mesh/moon/diffuse.jpg", ubo);

		UniformBufferObject ubo2{};
		ubo2.model = glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 1.0f);
		ubo2.model = glm::scale(ubo2.model, glm::vec3(0.1f, 0.1f, 0.1f));
		ubo2.view = glm::lookAt(glm::vec3(0.0f, -0.5f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo2.proj = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 10.0f);
		ubo2.proj[1][1] *= -1;

		UniformBufferObject ubo3{};
		ubo3.model = glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 1.0f);
		ubo3.model = glm::scale(ubo3.model, glm::vec3(0.2f, 0.2f, 0.2f));
		ubo3.model *= glm::translate(glm::mat4(1.0f), glm::vec3(-6.0f, 0.0f, 0.0f));
		ubo3.view = glm::lookAt(glm::vec3(0.0f, -0.5f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo3.proj = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 10.0f);
		ubo3.proj[1][1] *= -1;

		//AddMesh("mesh/moon/moon.obj", "mesh/moon/diffuse.jpg", ubo3);
		AddMesh("mesh/viking/viking_room.obj", "mesh/viking/viking_room.png", ubo);
		AddMesh("mesh/moon/moon.obj", "mesh/moon/diffuse.jpg", ubo3);
		//AddMesh("mesh/kitty/kitty.obj", "mesh/kitty/diffuse.jpg", ubo3);
		//AddMesh("mesh/moon/moon.obj", "mesh/moon/diffuse.jpg", ubo2);
		//AddMesh("mesh/moon/moon.obj", "mesh/moon/diffuse.jpg", ubo2);
		//AddMesh("mesh/backpack/backpack.obj", "mesh/backpack/diffuse.png", ubo2);
	}

	void RenderManager::AddMesh(const char* path, const char* texturePath, UniformBufferObject ubo)
	{
		Mesh mesh;
		Rbk::TinyObjLoader::LoadMesh(mesh, path);
		m_Renderer->AddTexture(mesh, texturePath);
		m_Renderer->AddMesh(mesh, ubo);
	}

	void RenderManager::AddShader(std::string name, std::vector<char> vertShaderCode, std::vector<char> fragShaderCode)
	{
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