#pragma once
#include "Rebulk/GUI/ImGui/Im.h"
#include "ILayer.h"
#include "Rebulk/Renderer/Adapter/VulkanAdapter.h"
#include "Rebulk/GUI/Window.h"
#include "Rebulk/Application.h"

namespace Rbk
{
    class VulkanLayer : public ILayer
    {
    public:
        virtual void Init() override;
        void Destroy();
        void DisplayFpsCounter(double timeStep);
        void DisplayAPI(VkPhysicalDeviceProperties devicesProps);
        void Render(double timeStep, VkPhysicalDeviceProperties devicesProps);
        void DisplayOptions();
        void DisplayTextures();
        void DisplaySounds();
        void AddRenderAdapter(std::shared_ptr<VulkanAdapter> renderAdapter);
        void AddWindow(std::shared_ptr<Window> window) { m_Window = window; };
        void AddTextureManager(std::shared_ptr<TextureManager> textureManager) { m_TextureManager = textureManager; };
        void AddEntityManager(std::shared_ptr<EntityManager> entityManager) { m_EntityManager = entityManager; };
        void AddShaderManager(std::shared_ptr<ShaderManager> shaderManager) { m_ShaderManager = shaderManager; };
        void AddAudioManager(std::shared_ptr<AudioManager> audioManager) { m_AudioManager = audioManager; };

        bool m_DebugOpen = true;
        bool m_ShowGrid = true;
        bool m_LightOpen = true;
        bool m_FogOpen = true;
        bool m_HUDOpen = true;
        bool m_OtherOpen = true;
        bool m_AmbientOpen = true;
        bool m_Looping = true;

    private:
        bool m_VSync = false;
        bool m_ShowDemo = false;

        std::shared_ptr<VulkanAdapter> m_Adapter;
        std::shared_ptr<Window> m_Window;
        std::shared_ptr<TextureManager> m_TextureManager;
        std::shared_ptr<EntityManager> m_EntityManager;
        std::shared_ptr<ShaderManager> m_ShaderManager;
        std::shared_ptr<AudioManager> m_AudioManager;
    };
}
