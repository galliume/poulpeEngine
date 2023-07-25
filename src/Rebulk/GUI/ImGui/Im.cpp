#include "Im.hpp"

#include <backends/imgui_impl_vulkan.cpp>
#include <backends/imgui_impl_glfw.cpp>

#include <volk.h>

namespace Rbk {

    VkAllocationCallbacks* Im::s_Allocator = nullptr;
    ImGuiInfo Im::s_ImGuiInfo;

    static void check_vk_result(VkResult err)
    {
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
    }

    void Im::Init(GLFWwindow* window, ImGuiInfo initInfo)
    {
        s_ImGuiInfo = std::move(initInfo);
        s_ImGuiInfo.info.Allocator = s_Allocator;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        io.Fonts->AddFontFromFileTTF(std::filesystem::absolute("./assets/fonts/montserrat/Montserrat-Regular.ttf").string().c_str(), 20.0f);
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        //io.LogFilename = "imguiLog.txt";
        //io.IniFilename = "imgui.ini";

        io.ConfigDockingWithShift = false;
        io.ConfigViewportsNoAutoMerge = false;
        //io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;

        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        style.WindowRounding = 5.3f;
        style.FrameRounding = 2.3f;
        style.ScrollbarRounding = 0;

        //#5A7A0A 90 122 10
        const ImVec4 darkGreen = ImVec4(90/255.f, 122/255.f, 10/255.f, 1.f);
        //#83D350 131 211 80
        const ImVec4 lightGreen = ImVec4(131/255.f, 211/255.f, 80/255.f, 1.f);
        //#FAB036 250 176 54
        const ImVec4 darkOrange = ImVec4(250/255.f, 176/255.f, 54/255.f, 1.f);
        //#FDD48A 253 212 138
        const ImVec4 lightOrange = ImVec4(253/255.f, 212/255.f, 138/255.f, 1.f);
        //#513C2F 81 60 47
        const ImVec4 darkBrown = ImVec4(81/255.f, 60/255.f, 47/255.f, 1.f);
        const ImVec4 white = ImVec4(1.f, 1.f, 1.f, 1.f);
        const ImVec4 black = ImVec4(0.f, 0.f, 0.f, 1.f);

        style.Colors[ImGuiCol_Text] = black;
        style.Colors[ImGuiCol_TextDisabled] = white;
        style.Colors[ImGuiCol_WindowBg] = lightOrange;
        style.Colors[ImGuiCol_ChildBg] = darkGreen;
        style.Colors[ImGuiCol_PopupBg] = darkGreen;
        style.Colors[ImGuiCol_Border] = lightOrange;
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.01f, 1.00f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.90f, 0.80f, 0.80f, 0.40f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.90f, 0.65f, 0.65f, 0.45f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.83f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.40f, 0.40f, 0.80f, 0.20f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.87f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.01f, 0.01f, 0.02f, 0.80f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.55f, 0.53f, 0.55f, 0.51f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.56f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.91f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.99f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 0.83f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.70f, 0.70f, 0.70f, 0.62f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.30f, 0.30f, 0.30f, 0.84f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.48f, 0.72f, 0.89f, 0.49f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.50f, 0.69f, 0.99f, 0.68f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.80f, 0.50f, 0.50f, 1.00f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.30f, 0.69f, 1.00f, 0.53f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.44f, 0.61f, 0.86f, 1.00f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.38f, 0.62f, 0.83f, 1.00f);
        style.Colors[ImGuiCol_Separator] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.70f, 0.60f, 0.60f, 1.00f);
        style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.90f, 0.70f, 0.70f, 1.00f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.85f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
        style.Colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

        ImGui_ImplVulkan_LoadFunctions(
            [](const char* name, void*) {
                return vkGetInstanceProcAddr(volkGetLoadedInstance(), name);
            }
        );

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
