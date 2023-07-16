#include "VulkanLayer.hpp"
#include "Rebulk/Application.hpp"

namespace Rbk
{
    void VulkanLayer::Init(Window* window, std::shared_ptr<CommandQueue> cmdQueue, VkPhysicalDeviceProperties deviceProperties)
    {
        m_CmdQueue = cmdQueue;
        m_deviceProperties = deviceProperties;

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
    }

    void VulkanLayer::Draw()
    {
        uint32_t frameIndex = m_RenderManager->GetRendererAdapter()->GetCurrentFrameIndex();

        m_RenderManager->GetRendererAdapter()->Rdr()->BeginCommandBuffer(Rbk::Im::s_ImGuiInfo.cmdBuffer);
        m_RenderManager->GetRendererAdapter()->Rdr()->BeginRenderPass(
            Rbk::Im::s_ImGuiInfo.rdrPass,
            Rbk::Im::s_ImGuiInfo.cmdBuffer,
            Rbk::Im::s_ImGuiInfo.frameBuffers[frameIndex]
        );
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), Rbk::Im::s_ImGuiInfo.cmdBuffer, Rbk::Im::s_ImGuiInfo.pipeline);
        m_RenderManager->GetRendererAdapter()->Rdr()->EndRenderPass(Rbk::Im::s_ImGuiInfo.cmdBuffer);
        m_RenderManager->GetRendererAdapter()->Rdr()->EndCommandBuffer(Rbk::Im::s_ImGuiInfo.cmdBuffer);

        std::vector<VkSemaphore>& imageAvailableSemaphores =  Rbk::Im::s_ImGuiInfo.semaphores.first;
            
        vkResetFences(m_RenderManager->GetRendererAdapter()->Rdr()->GetDevice(), 1, &m_Fence);

        VkResult result = vkAcquireNextImageKHR(
            m_RenderManager->GetRendererAdapter()->Rdr()->GetDevice(),
            m_RenderManager->GetRendererAdapter()->GetSwapChain(),
            UINT32_MAX,
            imageAvailableSemaphores[frameIndex],
            m_Fence,
            &m_ImGuiImageIndex);

        vkWaitForFences(m_RenderManager->GetRendererAdapter()->Rdr()->GetDevice(), 1, &m_Fence, VK_TRUE, UINT32_MAX);

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            RBK_ERROR("failed to acquire swap chain image!");
        }

        std::vector<VkSemaphore>& renderFinishedSemaphores = Rbk::Im::s_ImGuiInfo.semaphores.second;

        if (Rbk::Im::s_ImGuiInfo.imagesInFlight[m_ImGuiImageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(m_RenderManager->GetRendererAdapter()->Rdr()->GetDevice(), 1, &Rbk::Im::s_ImGuiInfo.imagesInFlight[m_ImGuiImageIndex], VK_TRUE, UINT64_MAX);
        }

        Rbk::Im::s_ImGuiInfo.imagesInFlight[m_ImGuiImageIndex] = Rbk::Im::s_ImGuiInfo.imagesInFlight[frameIndex];

        std::vector<VkSubmitInfo> submits{};

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[frameIndex] };

        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &Rbk::Im::s_ImGuiInfo.cmdBuffer;

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[frameIndex] };

        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        submits.emplace_back(submitInfo);
        
        vkResetFences(m_RenderManager->GetRendererAdapter()->Rdr()->GetDevice(), 1, &Rbk::Im::s_ImGuiInfo.inFlightFences[frameIndex]);

        result = vkQueueSubmit(m_RenderManager->GetRendererAdapter()->Rdr()->GetGraphicsQueues()[0], submits.size(), submits.data(), Rbk::Im::s_ImGuiInfo.inFlightFences[frameIndex]);
        vkWaitForFences(m_RenderManager->GetRendererAdapter()->Rdr()->GetDevice(), 1, &Rbk::Im::s_ImGuiInfo.inFlightFences[frameIndex], VK_TRUE, UINT32_MAX);

        vkResetCommandBuffer(Rbk::Im::s_ImGuiInfo.cmdBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

        renderFinishedSemaphores = Rbk::Im::s_ImGuiInfo.semaphores.second;

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { m_RenderManager->GetRendererAdapter()->GetSwapChain() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &m_ImGuiImageIndex;
        presentInfo.pResults = nullptr;

        VkResult err = vkQueuePresentKHR(m_RenderManager->GetRendererAdapter()->Rdr()->GetPresentQueue()[0], &presentInfo);
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

        if (m_Textures.empty() || m_Refresh) LoadTextures();

        ImGui::Begin("Rebulkan Engine", &open, flags);

            ImGuiID dockspace_id = ImGui::GetID("RebulkanDockspace");
            ImGui::DockSpace(dockspace_id, ImVec2(500.0f, 500.0f), dockspaceFlags);

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
                DisplayAPI(m_deviceProperties);
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

    void VulkanLayer::DisplayAPI(VkPhysicalDeviceProperties devicesProps)
    {
        Rbk::Im::Text("API Version : %s", m_RenderManager->GetRendererAdapter()->Rdr()->GetAPIVersion().c_str());
        //Rbk::Im::Text("Drivers version : %d", devicesProps.driverVersion);
        Rbk::Im::Text("Vendor id : %s", m_RenderManager->GetRendererAdapter()->Rdr()->GetVendor(devicesProps.vendorID).c_str());
        Rbk::Im::Text("GPU : %s", devicesProps.deviceName);
        ImGui::Separator();
        Rbk::Im::Text("Current frame %d", m_RenderManager->GetRendererAdapter()->Rdr()->GetCurrentFrame());
        ImGui::Separator();
        Rbk::Im::Text("%", "Meshes stats");
        Rbk::Im::Text("Total mesh loaded %d", m_RenderManager->GetEntityManager()->GetEntities()->size());
        Rbk::Im::Text("Total mesh instanced %d", m_RenderManager->GetEntityManager()->GetInstancedCount());
        ImGui::Separator();
        Rbk::Im::Text("Shader count %d", m_RenderManager->GetShaderManager()->GetShaders()->shaders.size());
        ImGui::Separator();
        Rbk::Im::Text("Texture count %d", m_RenderManager->GetTextureManager()->GetTextures().size());
        Rbk::Im::Text("%s", "Loaded textures :");
        for (auto tex : m_RenderManager->GetTextureManager()->GetTextures()) {
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
                m_RenderManager->GetAudioManager()->StartAmbient();
            }
            ImGui::SameLine();
            if (ImGui::Button("Stop"))
            {
                m_RenderManager->GetAudioManager()->StopAmbient();
            }
            ImGui::SameLine();
            if (ImGui::Checkbox("Loop", &m_Looping)) {
                m_RenderManager->GetAudioManager()->ToggleLooping();
            }

            ImGui::SameLine();

            Rbk::Im::Text("%s %s", 
                m_RenderManager->GetAudioManager()->GetState().c_str(),
                m_RenderManager->GetAudioManager()->GetCurrentAmbientSound().c_str()
            );

            ImGui::PushItemWidth(-1);
            if (ImGui::BeginListBox("##empty"))
            {
                for (int n = 0; n < m_RenderManager->GetAudioManager()->GetAmbientSound().size(); n++)
                {
                    const bool is_selected = (m_RenderManager->GetAudioManager()->GetAmbientSoundIndex() == n);
                    if (ImGui::Selectable(m_RenderManager->GetAudioManager()->GetAmbientSound()[n].c_str(), is_selected)) {
                        m_RenderManager->GetAudioManager()->StopAmbient();
                        m_RenderManager->GetAudioManager()->StartAmbient(n);
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
        std::vector<std::string> levels = m_RenderManager->GetConfigManager()->ListLevels();

        if (!m_LevelIndex.has_value()) {
            auto appConfig = m_RenderManager->GetConfigManager()->AppConfig();
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

        std::vector<std::string> skybox = m_RenderManager->GetConfigManager()->ListSkybox();

        if (ImGui::BeginCombo("Skybox", skybox.at(m_SkyboxIndex).c_str())) {

            for (int n = 0; n < skybox.size(); n++) {

                const bool isSelected = (m_SkyboxIndex == n);
                
                if (ImGui::Selectable(skybox.at(n).c_str(), isSelected)) {
                    m_SkyboxIndex = n;
                    m_Skybox = skybox.at(m_SkyboxIndex).c_str();
                    Refresh();
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if (ImGui::SmallButton("Reload")) Refresh();
    }

    void VulkanLayer::LoadTextures()
    {
        const auto& textures = m_RenderManager->GetTextureManager()->GetTextures();
        const auto& imageViews = m_RenderManager->GetRendererAdapter()->GetSwapChainImageViews();

        for (const auto& texture : textures) {
            
            if (!texture.second.IsPublic()) continue;

            VkDescriptorSet imgDset = ImGui_ImplVulkan_AddTexture(texture.second.GetSampler(), texture.second.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            m_Textures[texture.second.GetName()] = imgDset;
        }

        //for (const auto& imageView : *imageViews) {
        //    VkSampler textureSampler = m_RenderManager->GetRendererAdapter()->Rdr()->CreateTextureSampler(1);
        //    VkDescriptorSet imgDset = ImGui_ImplVulkan_AddTexture(textureSampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        //    m_Scenes.emplace_back(imgDset);
        //}
    }

    void VulkanLayer::AddRenderManager(RenderManager* renderManager)
    { 
        m_RenderManager = renderManager;
    }

    void VulkanLayer::Refresh()
    {
        //std::future<int> refresh = std::async(
        //    std::launch::async, [=, this](){ 
        //});

        m_RenderManager->Refresh(m_LevelIndex.value(), m_ShowBBox, m_Skybox);
        m_Refresh = true;
    }
}
