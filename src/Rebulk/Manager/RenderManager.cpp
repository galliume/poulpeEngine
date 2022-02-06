#include "RenderManager.h"

namespace Rbk
{
	RenderManager* RenderManager::s_Instance = nullptr;

	RenderManager::RenderManager(GLFWwindow* window, IRendererAdapter* renderer)
	{
		m_Renderer = renderer;

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

		AddMesh("mesh/moon/moon.obj", "mesh/moon/diffuse.jpg");
		AddMesh("mesh/backpack/backpack.obj", "mesh/backpack/diffuse.jpg");
	}

	void RenderManager::AddMesh(const char* path, const char* texturePath)
	{
		Mesh mesh;
		Rbk::TinyObjLoader::LoadMesh(mesh, path);

		m_Renderer->AddTexture(mesh, texturePath);
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