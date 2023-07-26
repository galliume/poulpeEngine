#include "VulkanLayer.hpp"
#include "Rebulk/Application.hpp"

namespace Rbk
{
    bool VulkanLayer::s_RenderViewportHasInput { false };

    void VulkanLayer::Init(Window* window, std::shared_ptr<CommandQueue> cmdQueue)
    {
        m_CmdQueue = cmdQueue;
        m_ImGuiInfo = std::make_shared<ImGuiInfo>(m_RenderManager->GetRendererAdapter()->GetImGuiInfo());
        Rbk::Im::Init(window->Get(), *m_ImGuiInfo);

        m_RenderManager->GetRendererAdapter()->ImmediateSubmit([&](VkCommandBuffer cmd) {
            ImGui_ImplVulkan_CreateFontsTexture(cmd);
        });

        ImGui_ImplVulkan_DestroyFontUploadObjects();

        m_ImGuiPool = m_RenderManager->GetRendererAdapter()->Rdr()->CreateCommandPool();

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        m_Fence;

        vkCreateFence(m_RenderManager->GetRendererAdapter()->Rdr()->GetDevice(), &fenceInfo, nullptr, &m_Fence);
        m_ImGuiImageIndex = 0;

        LoadDebugInfo();
        LoadTextures();
        LoadAmbiantSounds();
        LoadLevels();
        LoadSkybox();

        m_RenderManager->GetRendererAdapter()->AttachObserver(this);
    }

    void VulkanLayer::Notify(const Event& event)
    {
        if ("OnFinishRender" == event.name && m_OnFinishRender == false)
        {
            m_OnFinishRender = true;
            m_RenderScene = m_RenderManager->GetRendererAdapter()->GetImguiTexture();
            m_ImgDesc = ImGui_ImplVulkan_AddTexture(m_RenderScene.first, m_RenderScene.second, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            //m_DepthImage = m_RenderManager->GetRendererAdapter()->GetImguiDepthImage();
            //VkDescriptorSet depthImgDset = ImGui_ImplVulkan_AddTexture(m_DepthImage.first, m_DepthImage.second, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            //ImGui::Image(depthImgDset, ImVec2(my_tex_w, my_tex_h), uv_min, uv_max, tint_col, border_col);
        }
    }

    void VulkanLayer::LoadDebugInfo()
    {
        std::function<void()> request = [=, this]() {
            m_DebugInfo.deviceProperties = m_RenderManager->GetRendererAdapter()->Rdr()->GetDeviceProperties();
            m_DebugInfo.apiVersion = m_RenderManager->GetRendererAdapter()->Rdr()->GetAPIVersion();
            m_DebugInfo.vendorID = m_RenderManager->GetRendererAdapter()->Rdr()->GetVendor(m_DebugInfo.deviceProperties.vendorID);
            m_DebugInfo.totalMeshesLoaded = m_RenderManager->GetEntityManager()->GetEntities()->size();
            m_DebugInfo.totalMeshesInstanced = m_RenderManager->GetEntityManager()->GetInstancedCount();
            m_DebugInfo.totalShadersLoaded = m_RenderManager->GetShaderManager()->GetShaders()->shaders.size();
            m_DebugInfo.textures = m_RenderManager->GetTextureManager()->GetTextures();
        };

        Command cmd{request};

        m_CmdQueue->Add(cmd);
    }

    void VulkanLayer::LoadTextures()
    {
        std::function<void()> request = [=, this]() {
            
            std::unordered_map<std::string, VkDescriptorSet> tmpTextures{};

            const auto& textures = m_RenderManager->GetTextureManager()->GetTextures();
            const auto& imageViews = m_RenderManager->GetRendererAdapter()->GetSwapChainImageViews();

            for (const auto& texture : textures) {
            
                if (!texture.second.IsPublic()) continue;

                VkDescriptorSet imgDset = ImGui_ImplVulkan_AddTexture(texture.second.GetSampler(), texture.second.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

                tmpTextures[texture.second.GetName()] = imgDset;
            }

            std::swap(tmpTextures, m_Textures);
        };

        Command cmd{request};
        m_CmdQueue->Add(cmd);

        //for (const auto& imageView : *imageViews) {
        //    VkSampler textureSampler = m_RenderManager->GetRendererAdapter()->Rdr()->CreateTextureSampler(1);
        //    VkDescriptorSet imgDset = ImGui_ImplVulkan_AddTexture(textureSampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        //    m_Scenes.emplace_back(imgDset);
        //}
    }

    void VulkanLayer::LoadAmbiantSounds()
    {
        std::function<void()> requestSounds = [=, this]() {
            m_AmbientSounds = m_RenderManager->GetAudioManager()->GetAmbientSound();
        };

        std::function<void()> requestIndex = [=, this]() {
            m_SoundIndex = m_RenderManager->GetAudioManager()->GetAmbientSoundIndex();
        };

        Command cmdSounds{requestSounds};
        Command cmdIndex{requestIndex};

        m_CmdQueue->Add(cmdSounds);
        m_CmdQueue->Add(cmdIndex);
    }

    void VulkanLayer::LoadLevels()
    {
        std::function<void()> request = [=, this]() {
            m_Levels = m_RenderManager->GetConfigManager()->ListLevels();
        };

        Command cmd{request};
        m_CmdQueue->Add(cmd);
    }

    void VulkanLayer::LoadSkybox()
    {
        std::function<void()> request = [=, this]() {
            m_Skyboxs = m_RenderManager->GetConfigManager()->ListSkybox();
        };

        Command cmd{request};
        m_CmdQueue->Add(cmd);
    }

    void VulkanLayer::Render(double timeStep)
    {
        Rbk::Im::NewFrame();
        ImGuiWindowFlags flags = 0;
        flags |= ImGuiWindowFlags_MenuBar;

        ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
        
        if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
            flags |= ImGuiWindowFlags_NoBackground;

        bool open = true;

        ImGui::Begin("Rebulkan Engine", &open, flags);

            ImGuiID dockspace_id = ImGui::GetID("RebulkanDockspace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspaceFlags);

            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("Rebulk"))
                {
                    if (ImGui::MenuItem("Quit", "Alt+F4")) 
                    {
                        m_RenderManager->GetWindow()->Quit();
                    }
                    ImGui::Separator();
                    ImGui::EndMenu();
                }
            }
            ImGui::EndMenuBar();

            ImGui::Begin("Performances stats");
                DisplayFpsCounter(timeStep);
                ImGui::Separator();
                DisplayAPI();
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

            ImGui::Begin("3D View");
                float my_tex_w = m_RenderManager->GetRendererAdapter()->Rdr()->GetSwapChainExtent().width;
                float my_tex_h = m_RenderManager->GetRendererAdapter()->Rdr()->GetSwapChainExtent().height;
                ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
                ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
                ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
                ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
                ImVec2 surface = ImVec2(my_tex_w, my_tex_h);

                ImGui::Image(m_ImgDesc, surface, uv_min, uv_max, tint_col, border_col);

                if (ImGui::InvisibleButton("##RenderViewportHasInput", surface)) {
                    s_RenderViewportHasInput = true;
                    RBK_WARN("CLICKED!");
                }
                ImGui::SetItemAllowOverlap();

                ImGuiIO& io = ImGui::GetIO();

                if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Escape))) {
                    s_RenderViewportHasInput = false; 
                    io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
                }
                if (s_RenderViewportHasInput) {
                    io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
                    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
                }
            ImGui::End();

        ImGui::End();

        if (!open) {
            m_RenderManager->GetWindow()->Quit();
        }

        Rbk::Im::Render();
    }

    void VulkanLayer::Destroy()
    {
        Rbk::Im::Destroy();
    }

    void VulkanLayer::DisplayTextures()
    {
        int x = 0;
        
        if (!ImGui::BeginTable("table1", 6)) return;
    
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
        
        //if (!m_Scenes.empty()) {
        //    float my_tex_w = 150;
        //    float my_tex_h = 150;
        //    ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
        //    ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
        //    ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
        //    ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white

        //    ImGui::Image(m_Scenes.at(m_RenderManager->GetRendererAdapter()->GetCurrentFrameIndex()), ImVec2(my_tex_w, my_tex_h), uv_min, uv_max, tint_col, border_col);
        //}
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

    void VulkanLayer::DisplayAPI()
    {
        Rbk::Im::Text("API Version : %s", m_DebugInfo.apiVersion.c_str());
        Rbk::Im::Text("Vendor id : %s", m_DebugInfo.vendorID.c_str());
        Rbk::Im::Text("GPU : %s", m_DebugInfo.deviceProperties.deviceName);
        ImGui::Separator();
        Rbk::Im::Text("%", "Meshes stats");
        Rbk::Im::Text("Total mesh loaded %d", m_DebugInfo.totalMeshesLoaded);
        Rbk::Im::Text("Total mesh instanced %d", m_DebugInfo.totalMeshesInstanced);
        ImGui::Separator();
        Rbk::Im::Text("Shader count %d", m_DebugInfo.totalShadersLoaded);
        ImGui::Separator();
        Rbk::Im::Text("Texture count %d", m_DebugInfo.textures.size());
        for (auto tex : m_DebugInfo.textures) {
            Rbk::Im::Text("\t%s", tex.first.c_str());
        }
    }

    void VulkanLayer::DisplayOptions()
    {
        ImGui::SetNextItemOpen(m_DebugOpen);
        if ((m_DebugOpen = ImGui::CollapsingHeader("Debug")))
        {
            Rbk::Im::Text("%s", "Polygon mode"); 
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
                m_RenderManager->GetRendererAdapter()->ShowGrid(m_ShowGrid);
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

        m_RenderManager->GetWindow()->SetVSync(m_VSync);

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
                std::function<void()> request = [=, this]() {
                    m_RenderManager->GetAudioManager()->StartAmbient();
                };

                Command cmd{request};
                m_CmdQueue->Add(cmd);
            }

            ImGui::SameLine();

            if (ImGui::Button("Stop"))
            {
                std::function<void()> request = [=, this]() {
                    m_RenderManager->GetAudioManager()->StopAmbient();
                };

                Command cmd{request};
                m_CmdQueue->Add(cmd);
            }

            ImGui::SameLine();

            if (ImGui::Checkbox("Loop", &m_Looping)) {
                std::function<void()> request = [=, this]() {
                    m_RenderManager->GetAudioManager()->ToggleLooping();
                };

                Command cmd{request};
                m_CmdQueue->Add(cmd);
            }

            ImGui::SameLine();

            std::string state = m_RenderManager->GetAudioManager()->GetState();
            std::string currentAmbientSound = m_RenderManager->GetAudioManager()->GetCurrentAmbientSound();

            Rbk::Im::Text("%s %s", state.c_str(), currentAmbientSound.c_str());

            ImGui::PushItemWidth(-1);
            if (ImGui::BeginListBox("##empty"))
            {
                for (int n = 0; n < m_AmbientSounds.size(); n++)
                {
                    const bool is_selected = (m_SoundIndex == n);
                    if (ImGui::Selectable(m_AmbientSounds[n].c_str(), is_selected)) {
                        m_SoundIndex = n;
                        std::function<void()> request = [=, this]() {
                            m_RenderManager->GetAudioManager()->StopAmbient();
                            m_RenderManager->GetAudioManager()->StartAmbient(m_SoundIndex);
                        };

                        Command cmd{request};
                        m_CmdQueue->Add(cmd);
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
        if (!m_LevelIndex.has_value()) {
            auto appConfig = m_RenderManager->GetConfigManager()->AppConfig();
            auto defaultLevel = static_cast<std::string>(appConfig["defaultLevel"]);
            for (int i = 0; i < m_Levels.size(); ++i) {
                if (m_Levels.at(i).c_str() == defaultLevel) {
                    m_LevelIndex = i;
                    break;
                }
            }
        }

        if (m_Levels.size() > 0 && ImGui::BeginCombo("Levels", m_Levels.at(m_LevelIndex.value()).c_str())) {

            for (int n = 0; n < m_Levels.size(); n++) {

                const bool isSelected = m_LevelIndex == n;

                if (ImGui::Selectable(m_Levels.at(n).c_str(), isSelected)) {
                    m_LevelIndex = n;
                    Refresh();
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }


        if (m_Skyboxs.size() > 0 && ImGui::BeginCombo("Skybox", m_Skyboxs.at(m_SkyboxIndex).c_str())) {

            for (int n = 0; n < m_Skyboxs.size(); n++) {

                const bool isSelected = (m_SkyboxIndex == n);
                
                if (ImGui::Selectable(m_Skyboxs.at(n).c_str(), isSelected)) {
                    m_SkyboxIndex = n;
                    m_Skybox = m_Skyboxs.at(m_SkyboxIndex).c_str();
                    Refresh();
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if (ImGui::SmallButton("Reload")) Refresh();
    }

    void VulkanLayer::AddRenderManager(RenderManager* renderManager)
    {
        m_RenderManager = renderManager;
    }

    void VulkanLayer::UpdateData()
    {
        LoadDebugInfo();
        LoadTextures();
        LoadAmbiantSounds();
        LoadLevels();
        LoadSkybox();
    }

    void VulkanLayer::Refresh()
    {
        //std::future<int> refresh = std::async(
        //    std::launch::async, [=, this](){ 
        //});

        m_RenderManager->Refresh(m_LevelIndex.value(), m_ShowBBox, m_Skybox);
        m_Refresh = true;
        // 
        //std::function<void()> request = [=, this]() {
        //  m_RenderManager->Refresh(m_LevelIndex.value(), m_ShowBBox, m_Skybox);
        //  m_Refresh = true;
        //};

        //Command cmd{ request };
        //m_CmdQueue->Add(cmd);
    }

    void VulkanLayer::OnKeyPressed()
    {
        
    }
}
