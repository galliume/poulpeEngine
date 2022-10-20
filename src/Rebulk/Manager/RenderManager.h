#pragma once
#include "IRenderManager.h"

namespace Rbk
{
    class RenderManager : IRenderManager
    {
    public:
        RenderManager(
            std::shared_ptr<Window> window,
            std::shared_ptr<VulkanAdapter> renderer,
            std::shared_ptr<ConfigManager> configManager,
            std::shared_ptr<InputManager> inputManager,
            std::shared_ptr<AudioManager> audioManager,
            std::shared_ptr<TextureManager> textureManager,
            std::shared_ptr<EntityManager> entityManager,
            std::shared_ptr<ShaderManager> shaderManager,
            std::shared_ptr<SpriteAnimationManager> spriteAnimationManager,
            std::shared_ptr<Rbk::DestroyManager> destroyManager,
            std::shared_ptr<Rbk::Camera> camera
        );
        ~RenderManager();

        virtual void Init() override;
        virtual void Draw() override;
        virtual void SetDeltatime(float deltaTime) override;
        virtual bool IsLoaded()  override { return m_IsLoaded; }
        virtual void SetIsLoaded(bool loaded = true) override { m_IsLoaded = loaded; }
        virtual void Refresh() override;

        virtual std::shared_ptr<Window> GetWindow() override { return  m_Window; }
        virtual std::shared_ptr<Rbk::Camera> GetCamera() override { return m_Camera; }
        virtual std::shared_ptr<Rbk::InputManager> GetInputManager() override { return m_InputManager; }
        virtual std::shared_ptr<Rbk::VulkanAdapter> GetRendererAdapter() override { return m_Renderer; }
        virtual std::shared_ptr<Rbk::TextureManager> GetTextureManager() override { return m_TextureManager; }
        virtual std::shared_ptr<Rbk::SpriteAnimationManager> GetSpriteAnimationManager() override { return m_SpriteAnimationManager; }
        virtual std::shared_ptr<Rbk::EntityManager> GetEntityManager() override { return m_EntityManager; }
        virtual std::shared_ptr<Rbk::ShaderManager> GetShaderManager() override { return m_ShaderManager; }
        virtual std::shared_ptr<Rbk::AudioManager> GetAudioManager() override { return m_AudioManager; }
        virtual std::shared_ptr<Rbk::ConfigManager> GetConfigManager() override { return m_ConfigManager; }
        virtual std::shared_ptr<Rbk::DestroyManager> GetDestroyManager() override { return m_DestroyManager; }
        virtual void RenderManager::CleanUp() override;

    private:
        void PrepareSplashScreen();
        void Prepare();
        void LoadData();

    private:
        std::shared_ptr<Window> m_Window;
        std::shared_ptr<VulkanAdapter> m_Renderer;
        std::shared_ptr<Rbk::Camera> m_Camera;
        std::shared_ptr<Rbk::InputManager> m_InputManager;
        std::shared_ptr<Rbk::TextureManager> m_TextureManager;
        std::shared_ptr<Rbk::SpriteAnimationManager> m_SpriteAnimationManager;
        std::shared_ptr<Rbk::EntityManager> m_EntityManager;
        std::shared_ptr<Rbk::ShaderManager> m_ShaderManager;
        std::shared_ptr<Rbk::AudioManager> m_AudioManager;
        std::shared_ptr<Rbk::ConfigManager> m_ConfigManager;
        std::shared_ptr<Rbk::DestroyManager> m_DestroyManager;

        bool m_IsLoaded = false;

        //@todo should not be impl
        std::vector<VkDescriptorPool> m_DescriptorPools;
    };
}
