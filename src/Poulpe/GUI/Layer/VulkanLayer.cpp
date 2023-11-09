#include "VulkanLayer.hpp"

#include "PoulpeEngineConfig.h"
#include "Poulpe/Application.hpp"
#include "Poulpe/Renderer/Vulkan/EntityFactory.hpp"

#include <chrono>
#include <thread>

namespace Poulpe
{
    bool VulkanLayer::s_RenderViewportHasInput { false };
    bool VulkanLayer::s_OpenAbout { false };

    void VulkanLayer::init(Window* window, std::shared_ptr<CommandQueue> cmdQueue)
    {
        m_CmdQueue = cmdQueue;
        m_ImGuiInfo = std::make_shared<ImGuiInfo>(m_RenderManager->getRendererAdapter()->getImGuiInfo());
        Poulpe::Im::init(window->get(), *m_ImGuiInfo);

        m_RenderManager->getRendererAdapter()->immediateSubmit([&](VkCommandBuffer cmd) {
            ImGui_ImplVulkan_CreateFontsTexture(cmd);
        });

        ImGui_ImplVulkan_DestroyFontUploadObjects();

        m_ImGuiPool = m_RenderManager->getRendererAdapter()->rdr()->createCommandPool();

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        vkCreateFence(m_RenderManager->getRendererAdapter()->rdr()->getDevice(), &fenceInfo, nullptr, &m_Fence);
        m_ImGuiImageIndex = 0;

        loadDebugInfo();
        loadTextures();
        loadAmbiantSounds();
        loadLevels();
        loadSkybox();

        m_RenderManager->getRendererAdapter()->attachObserver(this);
    }

    VkDescriptorSet VulkanLayer::getImgDesc()
    {
      m_RenderScene = m_RenderManager->getRendererAdapter()->getImguiTexture();
      VkDescriptorSet imgDesc = ImGui_ImplVulkan_AddTexture(m_RenderScene.first, m_RenderScene.second, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

      return imgDesc;
    }

    void VulkanLayer::notify(Event const & event)
    {
        if ("OnFinishRender" == event.name)
        {
            if (!m_ImgDescDone) {
                m_ImgDesc = std::move(getImgDesc());
                m_ImgDescDone = true;
            }
        }
    }

    void VulkanLayer::loadDebugInfo()
    {
        std::function<void()> request = [=, this]() {
            m_DebugInfo.deviceProperties = m_RenderManager->getRendererAdapter()->rdr()->getDeviceProperties();
            m_DebugInfo.apiVersion = m_RenderManager->getRendererAdapter()->rdr()->getAPIVersion();
            m_DebugInfo.vendorID = m_RenderManager->getRendererAdapter()->rdr()->getVendor(m_DebugInfo.deviceProperties.vendorID);
            m_DebugInfo.totalMeshesLoaded = m_RenderManager->getEntityManager()->getEntities()->size();
            m_DebugInfo.totalMeshesInstanced = m_RenderManager->getEntityManager()->getInstancedCount();
            m_DebugInfo.totalShadersLoaded = m_RenderManager->getShaderManager()->getShaders()->shaders.size();
            m_DebugInfo.textures = m_RenderManager->getTextureManager()->getTextures();
        };

        Command cmd{request};

        m_CmdQueue->add(cmd);
    }

    void VulkanLayer::loadTextures()
    {
        std::function<void()> request = [=, this]() {
            
            std::unordered_map<std::string, VkDescriptorSet> tmpTextures{};

            auto const & textures = m_RenderManager->getTextureManager()->getTextures();
            //const auto& imageViews = m_RenderManager->GetRendererAdapter()->GetSwapChainImageViews();

            for (auto const & texture : textures) {
            
                if (!texture.second.isPublic()) continue;

                VkDescriptorSet imgDset = ImGui_ImplVulkan_AddTexture(texture.second.getSampler(), texture.second.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

                tmpTextures[texture.second.getName()] = imgDset;
            }

            std::swap(tmpTextures, m_Textures);
        };

        Command cmd{request};
        m_CmdQueue->add(cmd);

        //for (const auto& imageView : *imageViews) {
        //    VkSampler textureSampler = m_RenderManager->GetRendererAdapter()->Rdr()->CreateTextureSampler(1);
        //    VkDescriptorSet imgDset = ImGui_ImplVulkan_AddTexture(textureSampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        //    m_Scenes.emplace_back(imgDset);
        //}
    }

    void VulkanLayer::loadAmbiantSounds()
    {
        std::function<void()> requestSounds = [=, this]() {
            m_AmbientSounds = m_RenderManager->getAudioManager()->getAmbientSound();
        };

        std::function<void()> requestIndex = [=, this]() {
            m_SoundIndex = m_RenderManager->getAudioManager()->getAmbientSoundIndex();
        };

        Command cmdSounds{requestSounds};
        Command cmdIndex{requestIndex};

        m_CmdQueue->add(cmdSounds);
        m_CmdQueue->add(cmdIndex);
    }

    void VulkanLayer::loadLevels()
    {
        std::function<void()> request = [=, this]() {
            m_Levels = m_RenderManager->getConfigManager()->listLevels();
        };

        Command cmd{request};
        m_CmdQueue->add(cmd);
    }

    void VulkanLayer::updateSkybox()
    {
        std::function<void()> request = [=, this]() {
            {
                std::condition_variable cv;
                auto loading = m_RenderManager->getTextureManager()->loadSkybox(m_Skyboxs.at(m_SkyboxIndex), cv);
                loading();

                auto skybox = m_RenderManager->getEntityManager()->getSkybox();
                skybox->setIsDirty();
                auto entity = std::make_unique<Skybox>(EntityFactory::create<Skybox>(
                    m_RenderManager->getRendererAdapter(),
                    m_RenderManager->getEntityManager(),
                    m_RenderManager->getShaderManager(),
                    m_RenderManager->getTextureManager()));

                skybox->m_DescriptorSets = entity->createDescriptorSet(skybox);

                cv.notify_one(); //useful?
            }
        };

        Command cmd{request};
        m_CmdQueue->add(cmd);
    }

    void VulkanLayer::updateLevel()
    {
      std::function<void()> request = [=, this]() {
        {
          auto const start = std::chrono::high_resolution_clock::now();

          m_RenderManager->refresh(m_LevelIndex.value(), m_ShowBBox, m_Skyboxs.at(m_SkyboxIndex));

          while (!m_RenderManager->isLoaded()) {
            //just loading.
          };

          auto const end = std::chrono::high_resolution_clock::now();
          std::chrono::duration<double, std::milli> const elapsed = end - start;

          PLP_DEBUG("Loaded {} in {}", m_Levels.at(m_LevelIndex.value()), elapsed);

          m_ImgDescDone = false;
        }
      };

      Command cmd{ request, WhenToExecute::POST_RENDERING };
      m_CmdQueue->add(cmd);
    }

    void VulkanLayer::loadSkybox()
    {
        std::function<void()> request = [=, this]() {
            m_Skyboxs = m_RenderManager->getConfigManager()->listSkybox();
        };

        Command cmd{request};
        m_CmdQueue->add(cmd);
    }

    void VulkanLayer::updateRenderMode(VkPolygonMode mode)
    {
        std::function<void()> request = [=, this]() {
            Poulpe::VulkanAdapter::s_PolygoneMode.store(mode);
            vkResetDescriptorPool(m_RenderManager->getRendererAdapter()->rdr()->getDevice(), m_ImGuiInfo->info.DescriptorPool, 0);
            m_ImgDescDone = false;
            m_RenderManager->forceRefresh();
        };

        Command cmd{request};
        m_CmdQueue->add(cmd);
    }

    void VulkanLayer::render(double timeStep)
    {
        //ImGuiIO& io = ImGui::GetIO();
        
        Poulpe::Im::newFrame();

        ImGuiWindowFlags flags = 0;
        flags |= ImGuiWindowFlags_MenuBar;
        flags |= ImGuiWindowFlags_NoBackground;

        bool open{ true };

        ImGui::Begin("PoulpeEngine", &open, flags);

            ImGuiID poulpeEngineDockspaceid = ImGui::GetID("PoulpeEngineDockspace");
            ImGui::DockSpace(poulpeEngineDockspaceid, ImVec2(0.0f, 0.0f));

            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("Poulpe"))
                {
                    if (ImGui::MenuItem("Quit", "Alt+F4")) 
                    {
                        m_RenderManager->getWindow()->quit();
                    }
                    //ImGui::Separator();
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Help"))
                {
                    if (ImGui::MenuItem("About")) 
                    {
                        s_OpenAbout = true;
                    }

                    //ImGui::Separator();
                    ImGui::EndMenu();
                }
            }
            ImGui::EndMenuBar();

            ImGui::Begin("Performances stats");
                displayFpsCounter(timeStep);
                ImGui::Separator();
                displayAPI();
                ImGui::Separator();
            ImGui::End();

            ImGui::Begin("Level");
                displayLevel();
            ImGui::End();

            ImGui::Begin("Options");
                displayOptions();
            ImGui::End();

            //ImGui::Begin("Mesh");
            //    DisplayMesh();
            //ImGui::End();

            ImGui::Begin("Textures");
                displayTextures();
            ImGui::End();

            ImGui::Begin("Sound");
                displaySounds();
            ImGui::End();

            ImGui::Begin("3D View");
                float my_tex_w = m_RenderManager->getRendererAdapter()->rdr()->getSwapChainExtent().width;
                float my_tex_h = m_RenderManager->getRendererAdapter()->rdr()->getSwapChainExtent().height;
                ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
                ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
                ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
                ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
                ImVec2 surface = ImVec2(my_tex_w, my_tex_h);

                if (m_ImgDescDone) ImGui::Image(m_ImgDesc, surface, uv_min, uv_max, tint_col, border_col);
            ImGui::End();

            if (s_OpenAbout) {
                ImGui::OpenPopup("About");

                ImVec2 center = ImGui::GetMainViewport()->GetCenter();
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                ImGui::SetNextWindowSize(ImVec2(800.f, 600.f));

                if (ImGui::BeginPopupModal("About", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

                    float my_tex_w = 150;
                    float my_tex_h = 150;
                    ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
                    ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
                    ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
                    ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
                    ImVec2 pos = ImVec2((ImGui::GetContentRegionAvail().x - 150.f) * 0.5f, (ImGui::GetContentRegionAvail().y - 150.f) * .05f);

                    ImGui::SetCursorPos(pos);
                    ImGui::Image(m_Textures["mpoulpe"], ImVec2(my_tex_w, my_tex_h), uv_min, uv_max, tint_col, border_col);

                    ImGui::Text("%s %d.%d", "PoulpeEngine version", PoulpeEngine_VERSION_MAJOR, PoulpeEngine_VERSION_MINOR);
                    ImGui::Separator();
                    ImGui::Text("%s", "Build with C++ & Vulkan");
                    ImGui::Separator();
                    ImGui::Text("%s", "Third Parties:");
                    ImGui::NewLine();
                    ImGui::NewLine();
                    std::array<std::string, 9> thirdParties{
                        "GLFW", "GLM", "Dear ImGui", "MiniAudio", "Nlohmann json", "SpdLog", "STB", "Tiny Obj Loader", "Volk"};

                    std::array<std::string, 9> thirdPartiesURL{
                        "https://www.glfw.org/",
                        "https://github.com/g-truc/glm",
                        "https://github.com/ocornut/imgui",
                        "https://miniaud.io/",
                        "https://github.com/nlohmann/json",
                        "https://github.com/gabime/spdlog",
                        "https://github.com/nothings/stb",
                        "https://github.com/tinyobjloader/tinyobjloader",
                        "https://github.com/zeux/volk"};

                     if (ImGui::BeginTable("thirdParties", 2)) {
                        for (int row = 0; row < 9; row++) {
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGui::Text("%s", thirdParties.at(row).c_str());
                            ImGui::Separator();
                            ImGui::TableNextColumn();
                            ImGui::Text("%s", thirdPartiesURL.at(row).c_str());
                            ImGui::Separator();
                        }
                        ImGui::EndTable();
                    }

                    if (ImGui::Button("Close")) {
                        ImGui::CloseCurrentPopup();
                        s_OpenAbout = false;
                    }
                    ImGui::EndPopup();
                }
            }
        ImGui::End();

        if (!open) {
            m_RenderManager->getWindow()->quit();
        }

        Poulpe::Im::render();
    }

    void VulkanLayer::destroy()
    {
        Poulpe::Im::destroy();
    }

    void VulkanLayer::displayTextures()
    {
        int x = 0;
        
        if (!ImGui::BeginTable("table1", 6)) return;
    
        for (auto const & texture : m_Textures) {

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

            ImGui::Text("\t%s", texture.first.c_str());

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

    void VulkanLayer::displayMesh()
    {
        int x = 0;

        ImGui::BeginTable("table2", 6);

        for (auto const & entity : m_Entities) {

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

            ImGui::Text("\t%s", entity.first.c_str());

            if (5 > x) {
                x++;
            }
            else {
                x = 0;
            }
        }

        ImGui::EndTable();
    }

    void VulkanLayer::displayFpsCounter(double timeStep)
    {
        ImGui::Text("Frametime : %.2f ms", timeStep);
    }

    void VulkanLayer::displayAPI()
    {
        ImGui::Text("API Version : %s", m_DebugInfo.apiVersion.c_str());
        ImGui::Text("Vendor id : %s", m_DebugInfo.vendorID.c_str());
        ImGui::Text("GPU : %s", m_DebugInfo.deviceProperties.deviceName);
        ImGui::Separator();
        ImGui::Text("%s", "Meshes stats");
        ImGui::Text("Total mesh loaded %d", m_DebugInfo.totalMeshesLoaded);
        ImGui::Text("Total mesh instanced %d", m_DebugInfo.totalMeshesInstanced);
        ImGui::Separator();
        ImGui::Text("Shader count %d", m_DebugInfo.totalShadersLoaded);
        ImGui::Separator();
        ImGui::Text("Texture count %llu", m_DebugInfo.textures.size());
        for (auto tex : m_DebugInfo.textures) {
            ImGui::Text("\t%s", tex.first.c_str());
        }
    }

    void VulkanLayer::displayOptions()
    {
        ImGui::SetNextItemOpen(m_DebugOpen);
        if ((m_DebugOpen = ImGui::CollapsingHeader("Debug")))
        {
            ImGui::Text("%s", "Polygon mode"); 
            ImGui::SameLine();

            auto pm = Poulpe::VulkanAdapter::s_PolygoneMode.load();
            if (ImGui::RadioButton("Fill", &pm, VK_POLYGON_MODE_FILL)) {
                updateRenderMode(VK_POLYGON_MODE_FILL);
            };
            ImGui::SameLine();
            if (ImGui::RadioButton("Line", &pm, VK_POLYGON_MODE_LINE)) {
                Poulpe::VulkanAdapter::s_PolygoneMode.store(VK_POLYGON_MODE_LINE);
                updateRenderMode(VK_POLYGON_MODE_LINE);
            };

            ImGui::SameLine();
            if (ImGui::RadioButton("Point", &pm, VK_POLYGON_MODE_POINT)) {
                Poulpe::VulkanAdapter::s_PolygoneMode.store(VK_POLYGON_MODE_POINT);
                updateRenderMode(VK_POLYGON_MODE_POINT);
            }

            if (ImGui::Checkbox("Display grid", &m_ShowGrid)) {
                m_RenderManager->getRendererAdapter()->showGrid(m_ShowGrid);
            }

            if (ImGui::Checkbox("Display bbox", &m_ShowBBox)) {
                //m_RenderManager->SetDrawBbox(m_ShowBBox);
            }

            ImGui::Text("FPS limit"); ImGui::SameLine();
            auto fps = Poulpe::Application::s_UnlockedFPS.load();
            ImGui::RadioButton("30 fps", &fps, 0); ImGui::SameLine();
            ImGui::RadioButton("60 fps", &fps, 1); ImGui::SameLine();
            ImGui::RadioButton("120 fps", &fps, 2); ImGui::SameLine();
            ImGui::RadioButton("unlocked", &fps, 3);
            if (fps != Poulpe::Application::s_UnlockedFPS.load()) {
                Poulpe::Application::s_UnlockedFPS.store(fps);
            }
        }

        ImGui::SetNextItemOpen(m_LightOpen);
        if ((m_LightOpen = ImGui::CollapsingHeader("Light")))
        {
            auto ambiant = Poulpe::VulkanAdapter::s_AmbiantLight.load();
            ImGui::SliderFloat("Ambiant light", &ambiant, 0.0f, 1.0f, "%.3f");
            if (ambiant != Poulpe::VulkanAdapter::s_AmbiantLight.load()) {
                Poulpe::VulkanAdapter::s_AmbiantLight.store(ambiant);
            }
        }

        ImGui::SetNextItemOpen(m_FogOpen);
        if ((m_FogOpen = ImGui::CollapsingHeader("Fog")))
        {
            auto fogD = Poulpe::VulkanAdapter::s_FogDensity.load();
            ImGui::SliderFloat("Fog density", &fogD, 0.0f, 1.0f, "%.3f");
            if (fogD != Poulpe::VulkanAdapter::s_FogDensity) {
                Poulpe::VulkanAdapter::s_FogDensity.store(fogD);
            }

            auto fogC = Poulpe::VulkanAdapter::s_FogColor->load();
            ImGui::ColorEdit3("Fog color", &fogC);
            if (fogC != Poulpe::VulkanAdapter::s_FogColor->load()) {
                Poulpe::VulkanAdapter::s_FogColor->store(fogC);
            }
        }

        ImGui::SetNextItemOpen(m_HUDOpen);
        if ((m_HUDOpen = ImGui::CollapsingHeader("HUD")))
        {
            ImGui::Text("Crosshair style");
            ImGui::SameLine();
            auto crossH = Poulpe::VulkanAdapter::s_Crosshair.load();

            ImGui::RadioButton("Style 1", &crossH, 0); ImGui::SameLine();
            ImGui::RadioButton("Style 2", &crossH, 1);

            if (crossH != Poulpe::VulkanAdapter::s_Crosshair.load()) {
                Poulpe::VulkanAdapter::s_Crosshair.store(crossH);
            }
        }

        ImGui::SetNextItemOpen(m_OtherOpen);
        if ((m_OtherOpen = ImGui::CollapsingHeader("Other")))
        {
            ImGui::Checkbox("Show ImGui demo", &m_ShowDemo);
        }

        m_RenderManager->getWindow()->setVSync(m_VSync);

        if (m_ShowDemo) {
            ImGui::ShowDemoWindow();
        }
    }

    void VulkanLayer::displaySounds()
    {
        ImGui::SetNextItemOpen(m_AmbientOpen);

        if ((m_AmbientOpen = ImGui::CollapsingHeader("Ambient")))
        {
            if (ImGui::Button("Play"))
            {
                std::function<void()> request = [=, this]() {
                    m_RenderManager->getAudioManager()->startAmbient();
                };

                Command cmd{request};
                m_CmdQueue->add(cmd);
            }

            ImGui::SameLine();

            if (ImGui::Button("Stop"))
            {
                std::function<void()> request = [=, this]() {
                    m_RenderManager->getAudioManager()->stopAmbient();
                };

                Command cmd{request};
                m_CmdQueue->add(cmd);
            }

            ImGui::SameLine();

            if (ImGui::Checkbox("Loop", &m_Looping)) {
                std::function<void()> request = [=, this]() {
                    m_RenderManager->getAudioManager()->toggleLooping();
                };

                Command cmd{request};
                m_CmdQueue->add(cmd);
            }

            ImGui::SameLine();

            std::string state = m_RenderManager->getAudioManager()->getState();
            std::string currentAmbientSound = m_RenderManager->getAudioManager()->getCurrentAmbientSound();

            ImGui::Text("%s %s", state.c_str(), currentAmbientSound.c_str());

            ImGui::PushItemWidth(-1);
            if (ImGui::BeginListBox("##empty"))
            {
                for (size_t n = 0; n < m_AmbientSounds.size(); ++n)
                {
                    bool const is_selected = std::cmp_equal(m_SoundIndex, n);
                    if (ImGui::Selectable(m_AmbientSounds[n].c_str(), is_selected)) {
                        m_SoundIndex = n;
                        std::function<void()> request = [=, this]() {
                            m_RenderManager->getAudioManager()->stopAmbient();
                            m_RenderManager->getAudioManager()->startAmbient(m_SoundIndex);
                        };

                        Command cmd{request};
                        m_CmdQueue->add(cmd);
                    }

                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndListBox();
            }
        }
    }

    void VulkanLayer::displayLevel()
    {
        if (!m_LevelIndex.has_value()) {
            auto appConfig = m_RenderManager->getConfigManager()->appConfig();
            auto defaultLevel = static_cast<std::string>(appConfig["defaultLevel"]);
            for (size_t i = 0; i < m_Levels.size(); ++i) {
                if (m_Levels.at(i).c_str() == defaultLevel) {
                    m_LevelIndex = i;
                    break;
                }
            }
        }

        if (m_Levels.size() > 0 && ImGui::BeginCombo("Levels", m_Levels.at(m_LevelIndex.value()).c_str())) {

            for (size_t n = 0; n < m_Levels.size(); n++) {

                bool const isSelected = m_LevelIndex == n;

                if (ImGui::Selectable(m_Levels.at(n).c_str(), isSelected)) {
                    m_LevelIndex = n;
                    updateLevel();
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }


        if (m_Skyboxs.size() > 0 && ImGui::BeginCombo("Skybox", m_Skyboxs.at(m_SkyboxIndex).c_str())) {

            for (size_t n = 0; n < m_Skyboxs.size(); n++) {

                bool const isSelected = std::cmp_equal(m_SkyboxIndex, n);
                
                if (ImGui::Selectable(m_Skyboxs.at(n).c_str(), isSelected)) {
                    m_SkyboxIndex = n;
                    updateSkybox();
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

    void VulkanLayer::addRenderManager(RenderManager* renderManager)
    {
        m_RenderManager = renderManager;
    }

    void VulkanLayer::updateData()
    {
        loadDebugInfo();
        loadTextures();
        loadAmbiantSounds();
        loadLevels();
        loadSkybox();
    }

    void VulkanLayer::onKeyPressed()
    {
        
    }
}
