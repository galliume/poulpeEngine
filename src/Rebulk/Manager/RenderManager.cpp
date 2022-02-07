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

		AddShader(vertShaderCode, fragShaderCode);

		int width, height;
		glfwGetWindowSize(m_Window, &width, &height);

		UniformBufferObject ubo{};
		//ubo.model = glm::scale(glm::mat4(1.0f), glm::vec3(0.4f, 0.4f, 0.4f));
		//ubo.view = glm::lookAt(glm::vec3(2.0f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//ubo.proj = glm::perspective(glm::radians(122.0f), 500.0f / 240.0f, 0.0f, 1.0f);

		//ubo.proj[1][1] *= -1;
		//AddMesh("mesh/moon/moon.obj", "mesh/moon/diffuse.jpg", ubo);

		ubo.model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		ubo.view = glm::lookAt(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		ubo.proj = glm::perspective(glm::radians(122.0f), 500.0f / 240.0f, 0.0f, 1.0f);

		ubo.proj[1][1] *= -1;

		AddMesh("mesh/backpack/backpack.obj", "mesh/backpack/diffuse.jpg", ubo);
	}

	void RenderManager::AddMesh(const char* path, const char* texturePath, UniformBufferObject ubo)
	{
		Mesh mesh;
		Rbk::TinyObjLoader::LoadMesh(mesh, path);

		m_Renderer->AddTexture(mesh, texturePath);
		m_Renderer->AddUniformObject(ubo);
		m_Renderer->AddMesh(mesh);
	}

	void RenderManager::AddShader(std::vector<char> vertShaderCode, std::vector<char> fragShaderCode)
	{
		m_Renderer->AddShader(vertShaderCode, fragShaderCode);
	}

	void RenderManager::Draw()
	{
		m_Renderer->Draw();
	}
}