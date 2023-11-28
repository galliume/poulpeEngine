#pragma once

#include "ILayer.hpp"

#include "Poulpe/Core/IObserver.hpp"

#include "Poulpe/GUI/ImGui/Im.hpp"

#include "Poulpe/Manager/RenderManager.hpp"

#include <optional>

namespace Poulpe
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

        virtual ~VulkanLayer() = default;

        void init(Window* window) override;
        void render(float timeStep) override;
        void addRenderManager(RenderManager* renderManager) override;
        void destroy();
        void displayFpsCounter(float timeStep);
        void displayAPI();
        void displayOptions();
        void displayTextures();
        void displaySounds();
        void displayLevel();
        void displayMesh();
        void loadTextures();
        void draw();
        void updateData();
        void updateSkybox();
        void updateLevel();
        void displayGraph();

        void attachObserver(IObserver* observer);

        ImGuiInfo* getImGuiInfo() { return m_ImGuiInfo.get(); };

        bool m_DebugOpen{ true };
        bool m_ShowGrid{ true };
        bool m_ShowBBox{ false };
        bool m_LightOpen{ true };
        bool m_FogOpen{ true };
        bool m_HUDOpen{ true };
        bool m_OtherOpen{ true };
        bool m_AmbientOpen{ true };
        bool m_Looping{ true };

        virtual void notify(Event const & event) override;

        void loadDebugInfo();
        void loadAmbiantSounds();
        void loadLevels();
        void loadSkybox();
        void updateRenderMode();
        void updateResolution();

    private:
      [[nodiscard]] VkDescriptorSet getImgDesc();

    private:
        bool m_VSync{ false };
        bool m_ShowDemo{ false };
        std::optional<uint32_t> m_LevelIndex{ 0 };
        uint32_t m_SkyboxIndex{ 0 };

        RenderManager* m_RenderManager;
        std::unordered_map<std::string, VkDescriptorSet> m_Textures{};
        std::vector<VkDescriptorSet> m_CmdQueueScenes;
        std::unordered_map<std::string, VkDescriptorSet> m_Entities;
        std::unique_ptr<ImGuiInfo> m_ImGuiInfo;

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
        static bool s_OpenAbout;
        bool m_ImgDescDone = false;
        unsigned int m_Resolution{ 3 };
        unsigned int m_Ratio{ 0 };
    };
}
