#include "rebulkpch.hpp"
#include "Im.hpp"
#include <volk.h>

namespace Rbk {

    ImGuiInfo Im::s_ImGuiInfo{};

    void Im::Init(GLFWwindow* window, ImGuiInfo initInfo)
    {
        s_ImGuiInfo = initInfo;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        io.Fonts->AddFontFromFileTTF("./assets/fonts/montserrat/Montserrat-Regular.ttf", 20.0f);

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        io.ConfigDockingWithShift = false;
        io.ConfigViewportsNoAutoMerge = true;
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;

        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        ImGui_ImplVulkan_LoadFunctions(
            [](const char* name, void*) {
                return vkGetInstanceProcAddr(volkGetLoadedInstance(), name);
            }
        );

       // ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
        ImGui_ImplGlfw_InitForVulkan(window, true);
        ImGui_ImplVulkan_Init(&s_ImGuiInfo.info, s_ImGuiInfo.rdrPass);
    }

    void Im::NewFrame()
    {
        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplVulkan_NewFrame();
        ImGui::NewFrame();
    }
    
    void Im::CreateFontsTexture(VkCommandBuffer commandBuffer)
    {
        ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    }

    void Im::Begin(std::string name, bool* p_open, ImGuiWindowFlags flags)
    {
        ImGui::Begin(name.c_str(), p_open, flags);
    }
    
    void Im::End()
    {
        ImGui::End();
    }

    void Im::EndFrame()
    {
        ImGui::EndFrame();
    }

    void Im::EndChild()
    {
        ImGui::EndChild();
    }

    void Im::Separator()
    {
        ImGui::Separator();
    }

    void Im::BeginChild(std::string str_id, const ImVec2& size_arg, bool border, ImGuiWindowFlags extra_flags)
    {
        ImGui::BeginChild(str_id.c_str(), size_arg, border, extra_flags);
    }

    void Im::Render()
    {
        ImGui::Render();

        ImGuiIO& io = ImGui::GetIO();

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    void Im::Destroy()
    {
        ImGui_ImplVulkan_DestroyFontUploadObjects();
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}