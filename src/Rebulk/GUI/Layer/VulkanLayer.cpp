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
            m_Window->Quit();
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
        ImGui::SetNextItemOpen(m_DebugOpen);
        if (m_DebugOpen = ImGui::CollapsingHeader("Debug"))
        {
            Rbk::Im::Text("Polygon mode"); 
            ImGui::SameLine();

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

            if (ImGui::Checkbox("Display grid", &m_ShowGrid)) {
                m_Adapter->ShowGrid(m_ShowGrid);
            }

            Rbk::Im::Text("FPS limit");
            ImGui::SameLine();
            ImGui::RadioButton("60 fps", &Rbk::Application::s_UnlockedFPS, 0); ImGui::SameLine();
            ImGui::RadioButton("120 fps", &Rbk::Application::s_UnlockedFPS, 1); ImGui::SameLine();
            ImGui::RadioButton("unlocked", &Rbk::Application::s_UnlockedFPS, 2);
        }

        ImGui::SetNextItemOpen(m_LightOpen);
        if (m_LightOpen = ImGui::CollapsingHeader("Light"))
        {
            ImGui::SliderFloat("Ambiant light", &Rbk::VulkanAdapter::s_AmbiantLight, 0.0f, 1.0f, "%.3f");
        }

        ImGui::SetNextItemOpen(m_FogOpen);
        if (m_FogOpen = ImGui::CollapsingHeader("Fog"))
        {
            ImGui::SliderFloat("Fog density", &Rbk::VulkanAdapter::s_FogDensity, 0.0f, 1.0f, "%.3f");
            ImGui::ColorEdit3("Fog color", Rbk::VulkanAdapter::s_FogColor);
        }

        ImGui::SetNextItemOpen(m_HUDOpen);
        if (m_HUDOpen = ImGui::CollapsingHeader("HUD"))
        {
            Rbk::Im::Text("Crosshair style");
            ImGui::SameLine();
            ImGui::RadioButton("Style 1", &Rbk::VulkanAdapter::s_Crosshair, 0); ImGui::SameLine();
            ImGui::RadioButton("Style 2", &Rbk::VulkanAdapter::s_Crosshair, 1);
        }

        ImGui::SetNextItemOpen(m_OtherOpen);
        if (m_OtherOpen = ImGui::CollapsingHeader("Other"))
        {
            ImGui::Checkbox("Show ImGui demo", &m_ShowDemo);
        }

        m_Window->SetVSync(m_VSync);

        if (m_ShowDemo) {
            ImGui::ShowDemoWindow();
        }
    }

    void VulkanLayer::DisplaySounds()
    {
        ImGui::SetNextItemOpen(m_AmbientOpen);

        if (m_AmbientOpen = ImGui::CollapsingHeader("Ambient"))
        {
            if (ImGui::Button("Play"))
            {
                m_AudioManager->StartAmbient();
            }
            ImGui::SameLine();
            if (ImGui::Button("Stop"))
            {
                m_AudioManager->StopAmbient();
            }
            ImGui::SameLine();
            if (ImGui::Checkbox("Loop", &m_Looping)) {
                m_AudioManager->ToggleLooping();
            }

            ImGui::SameLine();

            Rbk::Im::Text("%s %s", m_AudioManager->GetState().c_str(), m_AudioManager->GetCurrentAmbientSound().c_str());

            ImGui::PushItemWidth(-1);
            if (ImGui::BeginListBox("##empty"))
            {
                for (int n = 0; n < m_AudioManager->GetAmbientSound().size(); n++)
                {
                    const bool is_selected = (m_AudioManager->GetAmbientSoundIndex() == n);
                    if (ImGui::Selectable(m_AudioManager->GetAmbientSound()[n].c_str(), is_selected)) {
                        m_AudioManager->StopAmbient();
                        m_AudioManager->StartAmbient(n);
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
        std::vector<std::string> levels = m_ConfigManager->ListLevels();

        if (ImGui::BeginCombo("Levels", levels.at(m_LevelIndex).c_str())) {

            for (int n = 0; n < levels.size(); n++) {

                const bool isSelected = (m_LevelIndex == n);

                if (ImGui::Selectable(levels.at(n).c_str(), isSelected)) {
                    m_LevelIndex = n;
                    nlohmann::json levelConfig = m_ConfigManager->EntityConfig(levels.at(n));
                    m_Adapter->GetEntityManager()->Clear();
                    std::vector<std::future<void>> entityFutures = m_Adapter->GetEntityManager()->Load(levelConfig);

                    for (auto& future : entityFutures) {
                        future.wait();
                    }
                    m_Adapter->Refresh();
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine();
        if (ImGui::Button("Refresh level"))
        {
            nlohmann::json levelConfig = m_ConfigManager->EntityConfig(levels.at(m_LevelIndex));
            m_Adapter->GetEntityManager()->Clear();
            std::vector<std::future<void>> entityFutures = m_Adapter->GetEntityManager()->Load(levelConfig);

            for (auto& future : entityFutures) {
                future.wait();
            }
            m_Adapter->Refresh();
        }

        std::vector<std::string> skybox = m_ConfigManager->ListSkybox();

        if (ImGui::BeginCombo("Skybox", skybox.at(m_SkyboxIndex).c_str())) {

            for (int n = 0; n < skybox.size(); n++) {

                const bool isSelected = (m_LevelIndex == n);

                if (ImGui::Selectable(skybox.at(n).c_str(), isSelected)) {
                    m_SkyboxIndex = n;
                    nlohmann::json skyboxConfig = m_ConfigManager->TexturesConfig()["skybox"];

                    std::vector<std::string>skyboxImages;
                    for (auto& texture : skyboxConfig[skybox.at(n)].items()) {
                        skyboxImages.emplace_back(texture.value());
                    }

                    m_Adapter->GetTextureManager()->AddSkyBox(skyboxImages);
                    m_Adapter->Refresh();
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

    void VulkanLayer::AddRenderAdapter(std::shared_ptr<VulkanAdapter> renderAdapter)
    {
        m_Adapter = renderAdapter;
    }

    void VulkanLayer::AddTextureManager(std::shared_ptr<TextureManager> textureManager)
    {
        m_TextureManager = textureManager; 

        const auto& textures = m_TextureManager->GetTextures();

        for (const auto& texture : textures) {
            
            if (!texture.second.IsPublic()) continue;

            VkDescriptorSet imgDset = ImGui_ImplVulkan_AddTexture(texture.second.GetSampler(), texture.second.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            m_Textures[texture.second.GetName()] = imgDset;
        }
    }

    void VulkanLayer::AddEntityManager(std::shared_ptr<EntityManager> entityManager)
    {
        m_EntityManager = entityManager;

        //const auto& entities = *m_EntityManager->GetEntities();

        //for (const auto& entity : entities) {

        //    std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(entity);
        //    m_Entities[mesh->GetName()] = mesh->GetDescriptorSets()[0];
        //}
    }
}
