#include "rebulkpch.hpp"
#include "VulkanLayer.hpp"
#include "Rebulk/Application.hpp"

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
                        m_RenderManager.load()->GetWindow()->Quit();
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

            ImGui::Begin("Level");
            DisplayLevel();
            ImGui::End();

            ImGui::Begin("Options");
                DisplayOptions();
            ImGui::End();

            //ImGui::Begin("Mesh");
            //    DisplayMesh();
            //ImGui::End();

            ImGui::Begin("Textures");
                DisplayTextures();
            ImGui::End();

            ImGui::Begin("Sound");
                DisplaySounds();
            ImGui::End();
        ImGui::End();

        if (!open) {
            m_RenderManager.load()->GetWindow()->Quit();
        }
    }
    void VulkanLayer::Destroy()
    {
        Rbk::Im::Destroy();
    }

    void VulkanLayer::DisplayTextures()
    {
        int x = 0;
        
        ImGui::BeginTable("table1", 6);
    
        for (const auto& texture : m_Textures) {

            if (0 == x) {
                ImGui::TableNextRow();
            }

            ImGui::TableSetColumnIndex(x);

            float my_tex_w = 150;
            float my_tex_h = 150;
            ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
            ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
            ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
            ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white

            ImGui::Image(texture.second, ImVec2(my_tex_w, my_tex_h), uv_min, uv_max, tint_col, border_col);

            Rbk::Im::Text("\t%s", texture.first.c_str());

            if (5 > x) {
                x++;
            } else {
                x = 0;
            }
        }

        ImGui::EndTable();
    }

    void VulkanLayer::DisplayMesh()
    {
        int x = 0;

        ImGui::BeginTable("table2", 6);

        for (const auto& entity : m_Entities) {

            if (0 == x) {
                ImGui::TableNextRow();
            }

            ImGui::TableSetColumnIndex(x);

            float my_tex_w = 150;
            float my_tex_h = 150;
            ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
            ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
            ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
            ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white

            ImGui::Image(entity.second, ImVec2(my_tex_w, my_tex_h), uv_min, uv_max, tint_col, border_col);

            Rbk::Im::Text("\t%s", entity.first.c_str());

            if (5 > x) {
                x++;
            }
            else {
                x = 0;
            }
        }

        ImGui::EndTable();
    }

    void VulkanLayer::DisplayFpsCounter(double timeStep)
    {
        Rbk::Im::Text("Frametime : %.2f ms", timeStep);
    }

    void VulkanLayer::DisplayAPI(VkPhysicalDeviceProperties devicesProps)
    {
        Rbk::Im::Text("API Version : %s", m_RenderManager.load()->GetRendererAdapter()->Rdr()->GetAPIVersion().c_str());
        //Rbk::Im::Text("Drivers version : %d", devicesProps.driverVersion);
        Rbk::Im::Text("Vendor id : %s", m_RenderManager.load()->GetRendererAdapter()->Rdr()->GetVendor(devicesProps.vendorID).c_str());
        Rbk::Im::Text("GPU : %s", devicesProps.deviceName);
        ImGui::Separator();
        Rbk::Im::Text("Current frame %d", m_RenderManager.load()->GetRendererAdapter()->Rdr()->GetCurrentFrame());
        ImGui::Separator();
        Rbk::Im::Text("%", "Meshes stats");
        Rbk::Im::Text("Total mesh loaded %d", m_RenderManager.load()->GetEntityManager()->GetEntities()->size());
        Rbk::Im::Text("Total mesh instanced %d", m_RenderManager.load()->GetEntityManager()->GetInstancedCount());
        ImGui::Separator();
        Rbk::Im::Text("Shader count %d", m_RenderManager.load()->GetShaderManager()->GetShaders()->shaders.size());
        ImGui::Separator();
        Rbk::Im::Text("Texture count %d", m_RenderManager.load()->GetTextureManager()->GetTextures().size());
        Rbk::Im::Text("Loaded textures :");
        for (auto tex : m_RenderManager.load()->GetTextureManager()->GetTextures()) {
            Rbk::Im::Text("\t%s", tex.first.c_str());
        }
    }

    void VulkanLayer::DisplayOptions()
    {
        ImGui::SetNextItemOpen(m_DebugOpen);
        if ((m_DebugOpen = ImGui::CollapsingHeader("Debug")))
        {
            Rbk::Im::Text("Polygon mode"); 
            ImGui::SameLine();

            auto pm = Rbk::VulkanAdapter::s_PolygoneMode.load();
            if (ImGui::RadioButton("Fill", &pm, VK_POLYGON_MODE_FILL)) {
                Rbk::VulkanAdapter::s_PolygoneMode.store(VK_POLYGON_MODE_FILL);
                Refresh();
            };
            ImGui::SameLine();
            if (ImGui::RadioButton("Line", &pm, VK_POLYGON_MODE_LINE)) {
                Rbk::VulkanAdapter::s_PolygoneMode.store(VK_POLYGON_MODE_LINE);
                Refresh();
            };
            ImGui::SameLine();
            if (ImGui::RadioButton("Point", &pm, VK_POLYGON_MODE_POINT)) {
                Rbk::VulkanAdapter::s_PolygoneMode.store(VK_POLYGON_MODE_POINT);
                Refresh();
            }

            if (ImGui::Checkbox("Display grid", &m_ShowGrid)) {
                m_RenderManager.load()->GetRendererAdapter()->ShowGrid(m_ShowGrid);
            }

            if (ImGui::Checkbox("Display bbox", &m_ShowBBox)) {
                Refresh();
            }

            Rbk::Im::Text("FPS limit"); ImGui::SameLine();
            auto fps = Rbk::Application::s_UnlockedFPS.load();
            ImGui::RadioButton("30 fps", &fps, 0); ImGui::SameLine();
            ImGui::RadioButton("60 fps", &fps, 1); ImGui::SameLine();
            ImGui::RadioButton("120 fps", &fps, 2); ImGui::SameLine();
            ImGui::RadioButton("unlocked", &fps, 3);
            if (fps != Rbk::Application::s_UnlockedFPS.load()) {
                Rbk::Application::s_UnlockedFPS.store(fps);
            }
        }

        ImGui::SetNextItemOpen(m_LightOpen);
        if ((m_LightOpen = ImGui::CollapsingHeader("Light")))
        {
            auto ambiant = Rbk::VulkanAdapter::s_AmbiantLight.load();
            ImGui::SliderFloat("Ambiant light", &ambiant, 0.0f, 1.0f, "%.3f");
            if (ambiant != Rbk::VulkanAdapter::s_AmbiantLight.load()) {
                Rbk::VulkanAdapter::s_AmbiantLight.store(ambiant);
            }
        }

        ImGui::SetNextItemOpen(m_FogOpen);
        if ((m_FogOpen = ImGui::CollapsingHeader("Fog")))
        {
            auto fogD = Rbk::VulkanAdapter::s_FogDensity.load();
            ImGui::SliderFloat("Fog density", &fogD, 0.0f, 1.0f, "%.3f");
            if (fogD != Rbk::VulkanAdapter::s_FogDensity) {
                Rbk::VulkanAdapter::s_FogDensity.store(fogD);
            }

            auto fogC = Rbk::VulkanAdapter::s_FogColor->load();
            ImGui::ColorEdit3("Fog color", &fogC);
            if (fogC != Rbk::VulkanAdapter::s_FogColor->load()) {
                Rbk::VulkanAdapter::s_FogColor->store(fogC);
            }
        }

        ImGui::SetNextItemOpen(m_HUDOpen);
        if ((m_HUDOpen = ImGui::CollapsingHeader("HUD")))
        {
            Rbk::Im::Text("Crosshair style");
            ImGui::SameLine();
            auto crossH = Rbk::VulkanAdapter::s_Crosshair.load();

            ImGui::RadioButton("Style 1", &crossH, 0); ImGui::SameLine();
            ImGui::RadioButton("Style 2", &crossH, 1);

            if (crossH != Rbk::VulkanAdapter::s_Crosshair.load()) {
                Rbk::VulkanAdapter::s_Crosshair.store(crossH);
            }
        }

        ImGui::SetNextItemOpen(m_OtherOpen);
        if ((m_OtherOpen = ImGui::CollapsingHeader("Other")))
        {
            ImGui::Checkbox("Show ImGui demo", &m_ShowDemo);
        }

        m_RenderManager.load()->GetWindow()->SetVSync(m_VSync);

        if (m_ShowDemo) {
            ImGui::ShowDemoWindow();
        }
    }

    void VulkanLayer::DisplaySounds()
    {
        ImGui::SetNextItemOpen(m_AmbientOpen);

        if ((m_AmbientOpen = ImGui::CollapsingHeader("Ambient")))
        {
            if (ImGui::Button("Play"))
            {
                m_RenderManager.load()->GetAudioManager()->StartAmbient();
            }
            ImGui::SameLine();
            if (ImGui::Button("Stop"))
            {
                m_RenderManager.load()->GetAudioManager()->StopAmbient();
            }
            ImGui::SameLine();
            if (ImGui::Checkbox("Loop", &m_Looping)) {
                m_RenderManager.load()->GetAudioManager()->ToggleLooping();
            }

            ImGui::SameLine();

            Rbk::Im::Text("%s %s", 
                m_RenderManager.load()->GetAudioManager()->GetState().c_str(),
                m_RenderManager.load()->GetAudioManager()->GetCurrentAmbientSound().c_str()
            );

            ImGui::PushItemWidth(-1);
            if (ImGui::BeginListBox("##empty"))
            {
                for (int n = 0; n < m_RenderManager.load()->GetAudioManager()->GetAmbientSound().size(); n++)
                {
                    const bool is_selected = (m_RenderManager.load()->GetAudioManager()->GetAmbientSoundIndex() == n);
                    if (ImGui::Selectable(m_RenderManager.load()->GetAudioManager()->GetAmbientSound()[n].c_str(), is_selected)) {
                        m_RenderManager.load()->GetAudioManager()->StopAmbient();
                        m_RenderManager.load()->GetAudioManager()->StartAmbient(n);
                    }

                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndListBox();
            }
        }
    }

    void VulkanLayer::DisplayLevel()
    {
        std::vector<std::string> levels = m_RenderManager.load()->GetConfigManager()->ListLevels();

        if (!m_LevelIndex.has_value()) {
            auto appConfig = m_RenderManager.load()->GetConfigManager()->AppConfig();
            auto defaultLevel = static_cast<std::string>(appConfig["defaultLevel"]);
            for (int i = 0; i < levels.size(); ++i) {
                if (levels.at(i).c_str() == defaultLevel) {
                    m_LevelIndex = i;
                    break;
                }
            }
        }

        if (ImGui::BeginCombo("Levels", levels.at(m_LevelIndex.value()).c_str())) {

            for (int n = 0; n < levels.size(); n++) {

                const bool isSelected = m_LevelIndex == n;

                if (ImGui::Selectable(levels.at(n).c_str(), isSelected)) {
                    m_LevelIndex = n;
                    Refresh();
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        std::vector<std::string> skybox = m_RenderManager.load()->GetConfigManager()->ListSkybox();

        if (ImGui::BeginCombo("Skybox", skybox.at(m_SkyboxIndex).c_str())) {

            for (int n = 0; n < skybox.size(); n++) {

                const bool isSelected = (m_SkyboxIndex == n);
                
                if (ImGui::Selectable(skybox.at(n).c_str(), isSelected)) {
                    m_SkyboxIndex = n;
                    m_RenderManager.load()->GetTextureManager()->LoadSkybox(skybox.at(n))();
                    m_RenderManager.load()->GetEntityManager()->GetSkybox()->SetIsDirty(true);
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

    void VulkanLayer::LoadTextures()
    {
        const auto& textures = m_RenderManager.load()->GetTextureManager()->GetTextures();

        for (const auto& texture : textures) {
            
            if (!texture.second.IsPublic()) continue;

            VkDescriptorSet imgDset = ImGui_ImplVulkan_AddTexture(texture.second.GetSampler(), texture.second.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            m_Textures[texture.second.GetName()] = imgDset;
        }
    }

    void VulkanLayer::AddRenderManager(std::shared_ptr<RenderManager> renderManager)
    { 
        m_RenderManager = renderManager;
        const auto& textures = m_RenderManager.load()->GetTextureManager()->GetTextures();

        for (const auto& texture : textures) {

            if (!texture.second.IsPublic()) continue;

            VkDescriptorSet imgDset = ImGui_ImplVulkan_AddTexture(texture.second.GetSampler(), texture.second.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            m_Textures[texture.second.GetName()] = imgDset;
        }

        m_Refresh = false;
    }

    void VulkanLayer::Refresh()
    {
        m_RenderManager.load()->Refresh(m_LevelIndex.value(), m_ShowBBox);
        m_Refresh = true;
    }
}
