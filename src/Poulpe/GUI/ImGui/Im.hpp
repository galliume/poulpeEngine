#pragma once

#include "imgui.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

namespace Poulpe
{
    struct ImGuiInfo
    {
        ImGui_ImplVulkan_InitInfo info = {};
        VkRenderPass rdrPass = nullptr;
        VkCommandBuffer cmdBuffer = nullptr;
        int width{ 0 };
        int height{ 0 };
        VkSurfaceKHR surface = nullptr;
        VkSurfaceFormatKHR surfaceFormat;
        VkPipeline pipeline = nullptr;
        VkPipelineLayout pipelineLayout = nullptr;
        VkDescriptorSet fontDescriptorSet = nullptr;
        std::vector<VkFramebuffer> frameBuffers {};
        std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> semaphores {};
        std::vector<VkFence> inFlightFences{};
        std::vector<VkFence> imagesInFlight{};
    };

    class Im
    {
    public:
        static void init(GLFWwindow* window, ImGuiInfo initInfo);
        static void newFrame();
        static void begin(std::string name, bool* p_open = NULL, ImGuiWindowFlags flags = 0);
        static void createFontsTexture(VkCommandBuffer commandBuffer);
        void static endChild();
        static void end();
        static void endFrame();
        static void separator();
        static void beginChild(std::string str_id, const ImVec2& size_arg, bool border, ImGuiWindowFlags extra_flags);
        static void render();
        static void destroy();
        static ImGuiInfo getImGuiInfo() { return s_ImGuiInfo; }
        static ImGuiInfo s_ImGuiInfo;
        static VkAllocationCallbacks* s_Allocator;
        static ImGui_ImplVulkanH_Window s_wd;
    };
}
