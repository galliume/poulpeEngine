#pragma once
#include "IRenderManager.h"

namespace Rbk
{
    class RenderManager : IRenderManager, public std::enable_shared_from_this<RenderManager>
    {
    public:
        explicit RenderManager(
            std::shared_ptr<Window> window,
            std::shared_ptr<IRendererAdapter> renderer,
            std::shared_ptr<IConfigManager> configManager,
            std::shared_ptr<IInputManager> inputManager,
            std::shared_ptr<IAudioManager> audioManager,
            std::shared_ptr<ITextureManager> textureManager,
            std::shared_ptr<IEntityManager> entityManager,
            std::shared_ptr<IShaderManager> shaderManager,
            std::shared_ptr<ISpriteAnimationManager> spriteAnimationManager,
            std::shared_ptr<Rbk::ILayerManager> layerManager,
            std::shared_ptr<Rbk::Camera>
        );
        ~RenderManager();

        virtual void Init() override;
        virtual void Draw() override;
        virtual void SetDeltatime(float deltaTime) override;
        virtual inline std::shared_ptr<IRendererAdapter> Adp()  override { return m_Renderer; };
        virtual bool IsLoaded()  override { return m_IsLoaded; };
        virtual void SetIsLoaded(bool loaded = true) override { m_IsLoaded = loaded; };

        virtual std::shared_ptr<Window> GetWindow() override { return  m_Window; };
        virtual std::shared_ptr<Rbk::Camera> GetCamera() override { return m_Camera; };
        virtual std::shared_ptr<Rbk::IInputManager> GetInputManager() override { m_InputManager; };
        virtual std::shared_ptr<Rbk::IRendererAdapter> GetRendererAdapter() override { m_RendererAdapter; };
        virtual std::shared_ptr<Rbk::ILayerManager> GetLayerManager() override { m_LayerManager; };
        virtual std::shared_ptr<Rbk::ITextureManager> GetTextureManager() override { m_TextureManager; };
        virtual std::shared_ptr<Rbk::ISpriteAnimationManager> GetSpriteAnimationManager() override { m_SpriteAnimationManager; };
        virtual std::shared_ptr<Rbk::IEntityManager> GetEntityManager() override { m_EntityManager; };
        virtual std::shared_ptr<Rbk::IShaderManager> GetShaderManager() override { m_ShaderManager; };
        virtual std::shared_ptr<Rbk::IAudioManager> GetAudioManager() override { m_AudioManager; }
        virtual std::shared_ptr<Rbk::IConfigManager> GetConfigManager() override { m_ConfigManager; };
        virtual std::shared_ptr<Rbk::ILayer> GetVulkanLayer() override { m_VulkanLayer; };

        inline static RenderManager& Get() { return *s_Instance; };

    private:
        static RenderManager* s_Instance;
        std::shared_ptr<IRendererAdapter> m_Renderer = nullptr;
        std::shared_ptr<Window> m_Window;
        std::shared_ptr<Rbk::Camera> m_Camera;
        std::shared_ptr<Rbk::IInputManager> m_InputManager;
        std::shared_ptr<Rbk::IRendererAdapter> m_RendererAdapter;
        std::shared_ptr<Rbk::ILayerManager> m_LayerManager;
        std::shared_ptr<Rbk::ITextureManager> m_TextureManager;
        std::shared_ptr<Rbk::ISpriteAnimationManager> m_SpriteAnimationManager;
        std::shared_ptr<Rbk::IEntityManager> m_EntityManager;
        std::shared_ptr<Rbk::IShaderManager> m_ShaderManager;
        std::shared_ptr<Rbk::IAudioManager> m_AudioManager;
        std::shared_ptr<Rbk::IConfigManager> m_ConfigManager;
        std::shared_ptr<Rbk::ILayer> m_VulkanLayer;

        bool m_IsLoaded = false;
    };
}