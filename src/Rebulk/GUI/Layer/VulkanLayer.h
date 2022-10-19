#pragma once
#include "ILayer.h"

namespace Rbk
{
    class VulkanLayer : public ILayer
    {
    public:
        virtual void Init() override;
        void Render(double timeStep, VkPhysicalDeviceProperties devicesProps) override;

        void Destroy();
        void DisplayFpsCounter(double timeStep);
        void DisplayAPI(VkPhysicalDeviceProperties devicesProps);
        void DisplayOptions();
        void DisplayTextures();
        void DisplaySounds();
        void DisplayLevel();
        void DisplayMesh();
        void AddRenderManager(std::shared_ptr<IRenderManager> renderManager) { m_RenderManager = renderManager; };
        void LoadTextures();

        bool m_DebugOpen = true;
        bool m_ShowGrid = true;
        bool m_ShowBBox = false;
        bool m_LightOpen = true;
        bool m_FogOpen = true;
        bool m_HUDOpen = true;
        bool m_OtherOpen = true;
        bool m_AmbientOpen = true;
        bool m_Looping = true;

    private:
        bool m_VSync = false;
        bool m_ShowDemo = false;
        int m_LevelIndex = 0;
        int m_SkyboxIndex = 0;

        std::shared_ptr<IRenderManager> m_RenderManager;
        std::map<std::string, VkDescriptorSet> m_Textures;
        std::map<std::string, VkDescriptorSet> m_Entities;
    };
}
