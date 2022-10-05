#include "rebulkpch.h"
#include "VulkanLayer.h"
#include <inttypes.h>

namespace Rbk
{
    void VulkanLayer::Init()
    {

    }

    void VulkanLayer::Render(double timeStep, VkPhysicalDeviceProperties devicesProps)
    {
        ImGuiWindowFlags flags = 0;
        flags |= ImGuiWindowFlags_MenuBar;

        bool open = true;

        ImGui::Begin("Rebulkan Engine", &open, flags);

            ImGuiID dockspace_id = ImGui::GetID("Dockspace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));

            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("Rebulk"))
                {
                    if (ImGui::MenuItem("Quit", "Alt+F4")) 
                    {
                        m_Window->Quit();
                    }
                    ImGui::Separator();
                    ImGui::EndMenu();
                }
            }
            ImGui::EndMenuBar();

            ImGui::Begin("Performances stats");
                DisplayFpsCounter(timeStep);
                ImGui::Separator();
                DisplayAPI(devicesProps);
                ImGui::Separator();
            ImGui::End();

            ImGui::Begin("Options");
                DisplayOptions();
            ImGui::End();

            ImGui::Begin("Textures");
                DisplayTextures();
            ImGui::End();
        ImGui::End();

        if (!open) {
            m_Window->Quit();
        }
    }
    void VulkanLayer::Destroy()
    {
        Rbk::Im::Destroy();
    }

    void VulkanLayer::DisplayTextures()
    {
        //ImGui::TreeNode("Grid");
        //int x = 0;
        //
        //ImGui::BeginTable("table1", 4);
        //const auto entities = m_Adapter->GetEntityManager()->GetEntities();

        //for (const auto entity : *entities) {

        //    std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(entity);
        //    if (!mesh) return;

        //    if (0 == x) {
        //        ImGui::TableNextRow();
        //    }

        //    ImGui::TableSetColumnIndex(x);

        //    float my_tex_w = 25;
        //    float my_tex_h = 25;
        //    ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
        //    ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
        //    ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
        //    ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
        //    for (const auto desc : mesh->GetDescriptorSets()) 
        //        ImGui::Image(desc, ImVec2(my_tex_w, my_tex_h), uv_min, uv_max, tint_col, border_col);

        //    Rbk::Im::Text("\t%s", mesh->m_Name.c_str());
        //    //Rbk::Im::Text("\t%s", path.c_str());

        //    if (3 > x) {
        //        x++;
        //    } else {
        //        x = 0;
        //    }
        //}

        //ImGui::EndTable();
    }

    void VulkanLayer::DisplayFpsCounter(double timeStep)
    {
        Rbk::Im::Text("FPS : %.2f", 1 / timeStep);
        Rbk::Im::Text("Frametime : %.2f ms", timeStep * 1000);	
    }

    void VulkanLayer::DisplayAPI(VkPhysicalDeviceProperties devicesProps)
    {                    
        Rbk::Im::Text("API Version : %s", m_Adapter->Rdr()->GetAPIVersion().c_str());
        //Rbk::Im::Text("Drivers version : %d", devicesProps.driverVersion);
        Rbk::Im::Text("Vendor id : %s", m_Adapter->Rdr()->GetVendor(devicesProps.vendorID).c_str());
        Rbk::Im::Text("GPU : %s", devicesProps.deviceName);
        ImGui::Separator();
        Rbk::Im::Text("Current frame %d", m_Adapter->Rdr()->GetCurrentFrame());
        ImGui::Separator();
        Rbk::Im::Text("Meshes stats");
        Rbk::Im::Text("Total mesh loaded %d", m_EntityManager->GetEntities()->size());
        Rbk::Im::Text("Total mesh instanced %d", m_EntityManager->GetInstancedCount());
        ImGui::Separator();
        Rbk::Im::Text("Shader count %d", m_ShaderManager->GetShaders()->shaders.size());
        ImGui::Separator();
        Rbk::Im::Text("Texture count %d", m_TextureManager->GetTextures().size());
        Rbk::Im::Text("Loaded textures :");
        for (auto tex : m_TextureManager->GetTextures()) {
            Rbk::Im::Text("\t%s", tex.first.c_str());
        }
    }

    void VulkanLayer::DisplayOptions()
    {
        Rbk::Im::Text("Polygon mode");

        if (ImGui::RadioButton("Fill", &Rbk::VulkanAdapter::s_PolygoneMode, VK_POLYGON_MODE_FILL)) {
            m_Adapter->Refresh();
        }; 
        ImGui::SameLine();
        if (ImGui::RadioButton("Line", &Rbk::VulkanAdapter::s_PolygoneMode, VK_POLYGON_MODE_LINE)) {
            m_Adapter->Refresh();
        }; 
        ImGui::SameLine();
        if (ImGui::RadioButton("Point", &Rbk::VulkanAdapter::s_PolygoneMode, VK_POLYGON_MODE_POINT)) {
            m_Adapter->Refresh();
        }
        ImGui::Separator();

        Rbk::Im::Text("Light");
        ImGui::SliderFloat("Ambiant light", &Rbk::VulkanAdapter::s_AmbiantLight, 0.0f, 1.0f, "%.3f");
        ImGui::Separator();
        Rbk::Im::Text("Fog");
        ImGui::SliderFloat("Fog density", &Rbk::VulkanAdapter::s_FogDensity, 0.0f, 1.0f, "%.3f");
        ImGui::ColorEdit3("Fog color", Rbk::VulkanAdapter::s_FogColor);
        ImGui::Separator();
        Rbk::Im::Text("Others");
        ImGui::RadioButton("60 fps", &Rbk::Application::s_UnlockedFPS, 0); ImGui::SameLine();
        ImGui::RadioButton("120 fps", &Rbk::Application::s_UnlockedFPS, 1); ImGui::SameLine();
        ImGui::RadioButton("unlocked", &Rbk::Application::s_UnlockedFPS, 2);

        ImGui::RadioButton("Crosshair 1", &Rbk::VulkanAdapter::s_Crosshair, 0); ImGui::SameLine();
        ImGui::RadioButton("Crosshair 2", &Rbk::VulkanAdapter::s_Crosshair, 1);

        ImGui::Separator();
        ImGui::Checkbox("Show ImGui demo", &m_ShowDemo);
        
        m_Adapter->SetWireFrameMode(m_WireframeModeOn);
        m_Window->SetVSync(m_VSync);

        if (m_ShowDemo) {
            ImGui::ShowDemoWindow();
        }
    }

    void VulkanLayer::AddRenderAdapter(std::shared_ptr<VulkanAdapter> renderAdapter)
    {
        m_Adapter = renderAdapter;
    }
}
