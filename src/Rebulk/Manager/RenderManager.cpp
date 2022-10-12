#include "rebulkpch.h"
#include "RenderManager.h"

namespace Rbk
{
    RenderManager* RenderManager::s_Instance = nullptr;

    RenderManager::RenderManager(
        std::shared_ptr<Window> window,
        std::shared_ptr<IRendererAdapter> renderer,
        std::shared_ptr<TextureManager> textureManager,
        std::shared_ptr<EntityManager> entityManager,
        std::shared_ptr<ShaderManager> shaderManager,
        std::shared_ptr<SpriteAnimationManager> spriteAnimationManager
    ) :
        m_Window(window),
        m_Renderer(renderer),
        m_TextureManager(textureManager),
        m_EntityManager(entityManager),
        m_ShaderManager(shaderManager),
        m_SpriteAnimationManager(spriteAnimationManager)
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
        m_Renderer->AddEntityManager(m_EntityManager);
        m_Renderer->AddShaderManager(m_ShaderManager);
        m_Renderer->AddSpriteAnimationManager(m_SpriteAnimationManager);
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