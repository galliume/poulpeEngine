#pragma once
#include "ILayer.h"
#include <volk.h>
#include "Rebulk/GUI/ImGui/Im.h"
#include "Rebulk/Manager/RenderManager.h"

namespace Rbk
{
    class VulkanLayer : public ILayer
    {
    public:
        virtual void Init() override;
        virtual void Render(double timeStep, VkPhysicalDeviceProperties devicesProps) override;
        virtual void AddRenderManager(std::shared_ptr<RenderManager> renderManager) override;

        void Destroy();
        void DisplayFpsCounter(double timeStep);
        void DisplayAPI(VkPhysicalDeviceProperties devicesProps);
        void DisplayOptions();
        void DisplayTextures();
        void DisplaySounds();
        void DisplayLevel();
        void DisplayMesh();
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
        void Refresh();

    private:
        bool m_VSync = false;
        bool m_ShowDemo = false;
        int m_LevelIndex = 0;
        int m_SkyboxIndex = 0;

        std::shared_ptr<RenderManager> m_RenderManager;
        std::map<std::string, VkDescriptorSet> m_Textures;
        std::map<std::string, VkDescriptorSet> m_Entities;
    };
}
