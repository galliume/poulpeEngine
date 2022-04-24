#include "rebulkpch.h"
#include "RenderManager.h"

namespace Rbk
{
    RenderManager* RenderManager::s_Instance = nullptr;

    RenderManager::RenderManager(GLFWwindow* window, IRendererAdapter* renderer, TextureManager* textureManager, MeshManager* meshManager, ShaderManager* shaderManager)
    {
        m_Window = window;
        m_Renderer = renderer;
        m_TextureManager = textureManager;
        m_MeshManager = meshManager;
        m_ShaderManager = shaderManager;

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
        m_Renderer->AddShaderManager(m_ShaderManager);
    }

    void RenderManager::AddCamera(Camera* camera)
    {
        m_Renderer->AddCamera(camera);
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