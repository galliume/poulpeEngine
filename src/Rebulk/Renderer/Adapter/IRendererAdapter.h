#pragma once

#include "Rebulk/Renderer/Vulkan/VulkanRenderer.h"
#include "Rebulk/Component/Camera.h"
#include "backends/imgui_impl_vulkan.h"
#include "Rebulk/GUI/ImGui/Im.h"

namespace Rbk
{
    class IRendererAdapter
    {
    public:
        virtual void Init() = 0;
        virtual void AddCamera(std::shared_ptr<Camera> camera) = 0;
        virtual void Draw() = 0;
        virtual void Destroy() = 0;
        virtual void SetDeltatime(float deltaTime) = 0;
        virtual void DrawSplashScreen() = 0;
        virtual void WaitIdle() = 0;
        virtual ImGuiInfo GetImGuiInfo() = 0;
        virtual void ShowGrid(bool show) = 0;
        virtual void AddEntities(std::vector<std::shared_ptr<Entity>>* entities) = 0;
        virtual void AddBbox(std::vector<std::shared_ptr<Entity>>* bbox) = 0;
        virtual void AddSkybox(std::shared_ptr<Mesh> skyboxMesh) = 0;
        virtual void AddHUD(std::vector<std::shared_ptr<Mesh>> hud) = 0;
        virtual void AddSplash(std::vector<std::shared_ptr<Mesh>> splash) = 0;
        virtual inline std::vector<VkDescriptorSetLayout>* GetDescriptorSetLayouts() = 0;
        virtual inline std::vector<VkImage>* GetSwapChainImages() = 0;
        virtual inline std::shared_ptr<VkRenderPass> RdrPass() = 0;
        virtual inline glm::mat4 GetPerspective() = 0;

        //@todo should return an interface not direct impl
        virtual void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function) = 0;
        virtual std::shared_ptr<VulkanRenderer> Rdr() = 0;
        //end @todo
    };
}