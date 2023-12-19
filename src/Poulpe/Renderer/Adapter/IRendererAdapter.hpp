#pragma once

#include "Poulpe/Component/Camera.hpp"
#include "Poulpe/Renderer/Vulkan/VulkanRenderer.hpp"

namespace Poulpe
{
    class IRendererAdapter
    {
    public:
        virtual void init() = 0;
        virtual void addCamera(Camera* camera) = 0;
        virtual void renderScene() = 0;
        virtual void draw() = 0;
        virtual void destroy() = 0;
        virtual void setDeltatime(float deltaTime) = 0;
        virtual void showGrid(bool show) = 0;
        virtual inline std::vector<VkDescriptorSetLayout>* getDescriptorSetLayouts() = 0;
        virtual inline std::vector<VkImage>* getSwapChainImages() = 0;
        virtual inline glm::mat4 getPerspective() = 0;

        //@todo should return an interface not direct impl
        virtual void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function, int queueIndex = 0) = 0;
        virtual VulkanRenderer* rdr() = 0;
        virtual void addPipeline(std::string const& shaderName, VulkanPipeline pipeline) = 0;
        virtual void prepareShadowMap() = 0;
        //end @todo
    };
}