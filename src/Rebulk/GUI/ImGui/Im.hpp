#pragma once

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

namespace Rbk
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
        static void Init(GLFWwindow* window, ImGuiInfo initInfo);
        static void NewFrame();
        static void Begin(std::string name, bool* p_open = NULL, ImGuiWindowFlags flags = 0);
        static void CreateFontsTexture(VkCommandBuffer commandBuffer);
        template <typename T, typename... Types>
        void static Text(T arg, Types... args) { ImGui::Text(arg, args...); };
        void static EndChild();
        static void End();
        static void EndFrame();
        static void Separator();
        static void BeginChild(std::string str_id, const ImVec2& size_arg, bool border, ImGuiWindowFlags extra_flags);
        static void Render();
        static void Destroy();
        static ImGuiInfo GetImGuiInfo() { return s_ImGuiInfo; }
        static ImGuiInfo s_ImGuiInfo;
        static VkAllocationCallbacks* s_Allocator;
        static ImGui_ImplVulkanH_Window s_wd;
    };
}
