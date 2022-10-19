#pragma once
#include "Rebulk/Component/Mesh.h"
#include "Rebulk/Component/Camera.h"
#include "Rebulk/Manager/ITextureManager.h"
#include "Rebulk/Manager/IEntityManager.h"
#include "Rebulk/Manager/IShaderManager.h"
#include "Rebulk/Manager/ISpriteAnimationManager.h"
#include "backends/imgui_impl_vulkan.h"

namespace Rbk
{
    struct ImGuiInfo
    {
        ImGui_ImplVulkan_InitInfo info = {};
        VkCommandBuffer cmdBuffer = nullptr;
        VkPipeline pipeline = nullptr;
        VkRenderPass rdrPass = nullptr;
    };

    class IRendererAdapter
    {
    public:
        virtual void Init() = 0;
        virtual void AddTextureManager(std::shared_ptr<ITextureManager> textureManager) = 0;
        virtual void AddEntityManager(std::shared_ptr<IEntityManager> EntityManager) = 0;
        virtual void AddShaderManager(std::shared_ptr<IShaderManager> shaderManager) = 0;
        virtual void AddSpriteAnimationManager(std::shared_ptr<ISpriteAnimationManager> spriteAnimationManager) = 0;
        virtual void AddCamera(std::shared_ptr<Camera> camera) = 0;
        virtual void Prepare() = 0;
        virtual void Draw() = 0;
        virtual void Destroy() = 0;
        virtual void SetDeltatime(float deltaTime) = 0;
        virtual void PrepareSplashScreen() = 0;
        virtual void DrawSplashScreen() = 0;
        virtual void WaitIdle() = 0;
        virtual ImGuiInfo GetImGuiInfo() = 0;
        virtual void Refresh() = 0;
        virtual void ShowGrid(bool show) = 0;

        //@todo should return an interface not direct impl
        virtual void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function) = 0;
        virtual std::shared_ptr<VulkanRenderer> Rdr() = 0;
        //end @todo
    };
}