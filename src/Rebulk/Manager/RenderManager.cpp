#include "rebulkpch.h"
#include "RenderManager.h"

namespace Rbk
{
    RenderManager* RenderManager::s_Instance = nullptr;

    RenderManager::RenderManager(
        std::shared_ptr<Window> window, 
        std::shared_ptr<IRendererAdapter> renderer, 
        std::shared_ptr<TextureManager> textureManager, 
        std::shared_ptr<MeshManager> meshManager, 
        std::shared_ptr<ShaderManager> shaderManager
    ) :
        m_Window(window),
        m_Renderer(renderer),
        m_TextureManager(textureManager),
        m_MeshManager(meshManager),
        m_ShaderManager(shaderManager)
    {
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

    void RenderManager::AddCamera(std::shared_ptr<Camera> camera)
    {
        m_Renderer->AddCamera(camera);
    }

    void RenderManager::Draw()
    {
        m_Renderer->Draw();
    }

    void RenderManager::SetDeltatime(float deltaTime)
    {
        m_Renderer->SetDeltatime(deltaTime);
    }
}