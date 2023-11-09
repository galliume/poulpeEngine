#include "Im.hpp"

#include <volk.h>

namespace Poulpe {

    VkAllocationCallbacks* Im::s_Allocator = nullptr;
    ImGuiInfo Im::s_ImGuiInfo;

    //@todo check why unused ?
    //static void check_vk_result(VkResult err)
    //{
    //    if (err == 0)
    //    {
    //        return;
    //    }

    //    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    //    if (err < 0)
    //    {
    //        abort();
    //    }
    //}

    void Im::init(GLFWwindow* window, ImGuiInfo initInfo)
    {
        s_ImGuiInfo = std::move(initInfo);
        s_ImGuiInfo.info.Allocator = s_Allocator;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        io.Fonts->AddFontFromFileTTF("./assets/fonts/montserrat/Montserrat-Regular.ttf", 20.0f);
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        
        //io.LogFilename = "imguiLog.txt";
        io.IniFilename = "./config/imgui.ini";

        io.ConfigDockingWithShift = false;
        io.ConfigViewportsNoAutoMerge = false;
        //io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;

        //ImGui::StyleColorsDark();

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        style.WindowRounding = 1.f;
        style.WindowBorderSize = 1.0f;
        style.FrameRounding = 0.f;
        style.FrameBorderSize = 1.f;
        style.ScrollbarRounding = 0;

        const ImVec4 white = ImVec4(1.f, 1.f, 1.f, 1.f);
        //const ImVec4 black = ImVec4(0.f, 0.f, 0.f, 1.f);
        const ImVec4 transparent = ImVec4(0.f, 0.f, 0.f, 0.f);
        //const ImVec4 darkGreen = ImVec4(76/255.f, 76/255.f, 26/255.f, 1.f);
        //const ImVec4 lightGreen = ImVec4(131/255.f, 211/255.f, 80/255.f, 1.f);
        //const ImVec4 darkOrange = ImVec4(250/255.f, 176/255.f, 54/255.f, 1.f);
        const ImVec4 lightOrange = ImVec4(253/255.f, 212/255.f, 138/255.f, 1.f);
        const ImVec4 darkGray = ImVec4(28/255.f, 28/255.f, 28/255.f, 1.f);
        const ImVec4 midDarkGray = ImVec4(52/255.f, 51/255.f, 51/255.f, 1.f);
        //const ImVec4 darkPurple = ImVec4(126/255.f, 92/255.f, 120/255.f, 1.f);
        //const ImVec4 lightPurple = ImVec4(163/255.f, 139/255.f, 160/255.f, 1.f);
        const ImVec4 darkBrown = ImVec4(41/255.f, 23/255.f, 15/255.f, 1.f);
        const ImVec4 midDarkBrown = ImVec4(79/255.f, 52/255.f, 30/255.f, 1.f);
        const ImVec4 goldenBrown = ImVec4(153/255.f, 106/255.f, 48/255.f, 1.f);
        const ImVec4 darkPink = ImVec4(186/255.f, 157/255.f, 111/255.f, 1.f);
        const ImVec4 lightPink = ImVec4(224/255.f, 211/255.f, 182/255.f, 1.f);
        const ImVec4 transparentLightPink = ImVec4(224/255.f, 211/255.f, 182/255.f, 0.2f);

        const ImVec4 text = white;
        const ImVec4 textDisabled = lightOrange;
        const ImVec4 windowBg = darkGray;
        const ImVec4 childBg = midDarkGray;
        const ImVec4 popupBg = midDarkGray;
        const ImVec4 border = goldenBrown;
        const ImVec4 borderShadow = darkBrown;
        const ImVec4 frameBg = midDarkGray;
        const ImVec4 frameBgHovered = goldenBrown;
        const ImVec4 frameBgActive = darkBrown;
        const ImVec4 titleBg = transparent;
        const ImVec4 titleBgCollapsed = darkBrown;
        const ImVec4 titleBgActive = goldenBrown;
        const ImVec4 menuBarBg = transparent;
        const ImVec4 scrollbarBg = darkBrown;
        const ImVec4 scrollbarGrab = goldenBrown;
        const ImVec4 scrollbarGrabHovered = goldenBrown;
        const ImVec4 scrollbarGrabActive = goldenBrown;
        const ImVec4 checkMark = goldenBrown;
        const ImVec4 sliderGrab = darkBrown;
        const ImVec4 sliderGrabActive = goldenBrown;
        const ImVec4 button = darkBrown;
        const ImVec4 buttonHovered = goldenBrown;
        const ImVec4 buttonActive = goldenBrown;
        const ImVec4 header = darkPink;
        const ImVec4 headerHovered = lightPink;
        const ImVec4 headerActive = lightPink;
        const ImVec4 separator = goldenBrown;
        const ImVec4 separatorHovered = midDarkBrown;
        const ImVec4 separatorActive = goldenBrown;
        const ImVec4 resizeGrip = goldenBrown;
        const ImVec4 resizeGripHovered = midDarkBrown;
        const ImVec4 resizeGripActive = goldenBrown;
        const ImVec4 plotLines = lightPink;
        const ImVec4 plotLinesHovered = darkPink;
        const ImVec4 plotHistogram = lightPink;
        const ImVec4 plotHistogramHovered = darkPink;
        const ImVec4 plotHistogramActive = lightPink;
        const ImVec4 modalWindowDimBg = transparentLightPink;
        const ImVec4 tab = lightPink;
        const ImVec4 tabHovered = lightPink;
        const ImVec4 tabActive = darkPink;
        const ImVec4 tabUnfocused = goldenBrown;
        const ImVec4 tabUnfocusedActive = darkPink;
        const ImVec4 dockingPreview = lightPink;
        const ImVec4 dockingEmptyBg = lightPink;

        style.Colors[ImGuiCol_Text] = text;
        style.Colors[ImGuiCol_TextDisabled] = textDisabled;
        style.Colors[ImGuiCol_WindowBg] = windowBg;
        style.Colors[ImGuiCol_ChildBg] = childBg;
        style.Colors[ImGuiCol_PopupBg] = popupBg;
        style.Colors[ImGuiCol_Border] = border;
        style.Colors[ImGuiCol_BorderShadow] = borderShadow;
        style.Colors[ImGuiCol_FrameBg] = frameBg;
        style.Colors[ImGuiCol_FrameBgHovered] = frameBgHovered;
        style.Colors[ImGuiCol_FrameBgActive] = frameBgActive;
        style.Colors[ImGuiCol_TitleBg] = titleBg;
        style.Colors[ImGuiCol_TitleBgCollapsed] = titleBgCollapsed;
        style.Colors[ImGuiCol_TitleBgActive] = titleBgActive;
        style.Colors[ImGuiCol_MenuBarBg] = menuBarBg;
        style.Colors[ImGuiCol_ScrollbarBg] = scrollbarBg;
        style.Colors[ImGuiCol_ScrollbarGrab] = scrollbarGrab;
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = scrollbarGrabHovered;
        style.Colors[ImGuiCol_ScrollbarGrabActive] = scrollbarGrabActive;
        style.Colors[ImGuiCol_CheckMark] = checkMark;
        style.Colors[ImGuiCol_SliderGrab] = sliderGrab;
        style.Colors[ImGuiCol_SliderGrabActive] = sliderGrabActive;
        style.Colors[ImGuiCol_Button] = button;
        style.Colors[ImGuiCol_ButtonHovered] = buttonHovered;
        style.Colors[ImGuiCol_ButtonActive] = buttonActive;
        style.Colors[ImGuiCol_Header] = header;
        style.Colors[ImGuiCol_HeaderHovered] = headerHovered;
        style.Colors[ImGuiCol_HeaderActive] = headerActive;
        style.Colors[ImGuiCol_Separator] = separator;
        style.Colors[ImGuiCol_SeparatorHovered] = separatorHovered;
        style.Colors[ImGuiCol_SeparatorActive] = separatorActive;
        style.Colors[ImGuiCol_ResizeGrip] = resizeGrip;
        style.Colors[ImGuiCol_ResizeGripHovered] = resizeGripHovered;
        style.Colors[ImGuiCol_ResizeGripActive] = resizeGripActive;
        style.Colors[ImGuiCol_PlotLines] = plotLines;
        style.Colors[ImGuiCol_PlotLinesHovered] = plotLinesHovered;
        style.Colors[ImGuiCol_PlotHistogram] = plotHistogram;
        style.Colors[ImGuiCol_PlotHistogramHovered] = plotHistogramHovered;
        style.Colors[ImGuiCol_TextSelectedBg] = plotHistogramActive;
        style.Colors[ImGuiCol_ModalWindowDimBg] = modalWindowDimBg;
        style.Colors[ImGuiCol_Tab] = tab;
        style.Colors[ImGuiCol_TabHovered] = tabHovered;
        style.Colors[ImGuiCol_TabActive] = tabActive;
        style.Colors[ImGuiCol_TabUnfocused] = tabUnfocused;
        style.Colors[ImGuiCol_TabUnfocusedActive] = tabUnfocusedActive;
        style.Colors[ImGuiCol_DockingPreview] = dockingPreview;
        style.Colors[ImGuiCol_DockingEmptyBg] = dockingEmptyBg;

        ImGui_ImplVulkan_LoadFunctions(
            [](const char* name, void*) {
                return vkGetInstanceProcAddr(volkGetLoadedInstance(), name);
            }
        );

        ImGui_ImplGlfw_InitForVulkan(window, true);
        ImGui_ImplGlfw_SetCallbacksChainForAllWindows(true);
        ImGui_ImplVulkan_Init(&s_ImGuiInfo.info, s_ImGuiInfo.rdrPass);
    }

    void Im::newFrame()
    {
        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplVulkan_NewFrame();
        ImGui::NewFrame();
    }

    void Im::createFontsTexture(VkCommandBuffer commandBuffer)
    {
        ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    }

    void Im::begin(std::string name, bool* p_open, ImGuiWindowFlags flags)
    {
        ImGui::Begin(name.c_str(), p_open, flags);
    }
    
    void Im::end()
    {
        ImGui::End();
    }

    void Im::endFrame()
    {
        ImGui::EndFrame();
    }

    void Im::endChild()
    {
        ImGui::EndChild();
    }

    void Im::separator()
    {
        ImGui::Separator();
    }

    void Im::beginChild(std::string str_id, const ImVec2& size_arg, bool border, ImGuiWindowFlags extra_flags)
    {
        ImGui::BeginChild(str_id.c_str(), size_arg, border, extra_flags);
    }

    void Im::render()
    {
        ImGui::Render();

        ImGuiIO& io = ImGui::GetIO();

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    void Im::destroy()
    {
        ImGui_ImplVulkan_DestroyFontUploadObjects();
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}
