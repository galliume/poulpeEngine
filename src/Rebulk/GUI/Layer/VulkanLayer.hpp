#pragma once
#include <optional>
#include "ILayer.hpp"
#include "Rebulk/GUI/ImGui/Im.hpp"
#include "Rebulk/Manager/RenderManager.hpp"

namespace Rbk
{
    class VulkanLayer : public ILayer
    {
    public:
        virtual void Init(Window* window, std::shared_ptr<CommandQueue> cmdQueue, VkPhysicalDeviceProperties deviceProperties) override;
        virtual void Render(double timeStep) override;
        virtual void AddRenderManager(RenderManager* renderManager) override;
        void SetNeedRefresh(bool needRefresh) { m_Refresh = needRefresh; };
        void Destroy();
        void DisplayFpsCounter(double timeStep);
        void DisplayAPI(VkPhysicalDeviceProperties devicesProps);
        void DisplayOptions();
        void DisplayTextures();
        void DisplaySounds();
        void DisplayLevel();
        void DisplayMesh();
        void LoadTextures();
        bool NeedRefresh() { return m_Refresh; }
        void Draw();
        std::shared_ptr<ImGuiInfo> GetImGuiInfo() { return m_ImGuiInfo; };

        bool m_DebugOpen = true;
        bool m_ShowGrid = true;
        bool m_ShowBBox = false;
        bool m_LightOpen = true;
        bool m_FogOpen = true;
        bool m_HUDOpen = true;
        bool m_OtherOpen = true;
        bool m_AmbientOpen = true;
        bool m_Looping = true;
        std::string_view m_Skybox{ "debug" };

    private:
        void Refresh();

    private:
        bool m_VSync = false;
        bool m_ShowDemo = false;
        std::optional<int> m_LevelIndex;
        int m_SkyboxIndex = 0;
        bool m_Refresh = false;

        RenderManager* m_RenderManager;
        std::unordered_map<std::string, VkDescriptorSet> m_Textures;
        std::vector<VkDescriptorSet> m_m_CmdQueueScenes;
        std::unordered_map<std::string, VkDescriptorSet> m_Entities;
        std::shared_ptr<CommandQueue> m_CmdQueue;
        std::shared_ptr<ImGuiInfo> m_ImGuiInfo;
        VkPhysicalDeviceProperties m_deviceProperties;

        VkCommandPool m_ImGuiPool;
        VkFence m_Fence;
        uint32_t m_ImGuiImageIndex;
    };
}
