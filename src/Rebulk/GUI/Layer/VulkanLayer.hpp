#pragma once
#include <optional>
#include "ILayer.hpp"
#include "Rebulk/GUI/ImGui/Im.hpp"
#include "Rebulk/Manager/RenderManager.hpp"
#include "Rebulk/Core/IObserver.hpp"

namespace Rbk
{
    struct DebugInfo
    {
        VkPhysicalDeviceProperties deviceProperties{};
        std::string apiVersion{};
        std::string vendorID{};
        uint32_t totalMeshesLoaded = 0;
        uint32_t totalMeshesInstanced = 0;
        uint32_t totalShadersLoaded = 0;
        std::unordered_map<std::string, Texture> textures{};
    };

    class VulkanLayer : public ILayer, public IObserver, public std::enable_shared_from_this<VulkanLayer>
    {
    public:
        virtual void Init(Window* window, std::shared_ptr<CommandQueue> cmdQueue) override;
        virtual void Render(double timeStep) override;
        virtual void AddRenderManager(RenderManager* renderManager) override;
        void SetNeedRefresh(bool needRefresh) { m_Refresh = needRefresh; };
        void Destroy();
        void DisplayFpsCounter(double timeStep);
        void DisplayAPI();
        void DisplayOptions();
        void DisplayTextures();
        void DisplaySounds();
        void DisplayLevel();
        void DisplayMesh();
        void LoadTextures();
        bool NeedRefresh() { return m_Refresh; }
        void Draw();
        std::shared_ptr<ImGuiInfo> GetImGuiInfo() { return m_ImGuiInfo; };
        void UpdateData();
        void AttachObserver(IObserver* observer);

        bool m_DebugOpen{ true };
        bool m_ShowGrid{ true };
        bool m_ShowBBox{ false };
        bool m_LightOpen{ true };
        bool m_FogOpen{ true };
        bool m_HUDOpen{ true };
        bool m_OtherOpen{ true };
        bool m_AmbientOpen{ true };
        bool m_Looping{ true };
        std::string_view m_Skybox{ "debug" };

        void LoadDebugInfo();
        void LoadAmbiantSounds();
        void LoadLevels();
        void LoadSkybox();
        virtual void Notify(const Event& event) override;
        void OnKeyPressed();

    private:
        void Refresh();

    private:
        bool m_VSync{ false };
        bool m_ShowDemo{ false };
        std::optional<uint32_t> m_LevelIndex{ 0 };
        uint32_t m_SkyboxIndex{ 0 };
        bool m_Refresh{ false };

        RenderManager* m_RenderManager;
        std::unordered_map<std::string, VkDescriptorSet> m_Textures{};
        std::vector<VkDescriptorSet> m_CmdQueueScenes;
        std::unordered_map<std::string, VkDescriptorSet> m_Entities;
        std::shared_ptr<CommandQueue> m_CmdQueue;
        std::shared_ptr<ImGuiInfo> m_ImGuiInfo;

        VkCommandPool m_ImGuiPool;
        VkFence m_Fence;
        uint32_t m_ImGuiImageIndex{0};
        DebugInfo m_DebugInfo;
        std::vector<std::string> m_AmbientSounds {};
        uint32_t m_SoundIndex{0};
        std::vector<std::string> m_Levels {};
        std::vector<std::string> m_Skyboxs {};
        std::pair<VkSampler, VkImageView> m_RenderScene;
        std::pair<VkSampler, VkImageView> m_DepthImage;
        VkDescriptorSet m_ImgDesc;

        bool m_OnFinishRender{ false };
        static bool s_RenderViewportHasInput;

        std::vector<IObserver> m_Observers{};
    };
}
