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

        ImGui::End();

        if (!open) {
            m_Window->Quit();
        }
    }
    void VulkanLayer::Destroy()
    {
        Rbk::Im::Destroy();
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
        Rbk::Im::Text("Vendor id : %s", m_Adapter->Rdr()->GetVendor(devicesProps.vendorID));
        Rbk::Im::Text("GPU : %s", devicesProps.deviceName);
        ImGui::Separator();
        Rbk::Im::Text("Current frame %d", m_Adapter->Rdr()->GetCurrentFrame());
        ImGui::Separator();
        Rbk::Im::Text("World meshes stats");
        Rbk::Im::Text("Total mesh loaded %d", m_MeshManager->GetWorldMeshes()->size());
        Rbk::Im::Text("Total mesh instanced %d", m_MeshManager->GetWorldInstancedCount());
        Rbk::Im::Text("Total vertex count %d", m_MeshManager->GetWorldVerticesCount());
        Rbk::Im::Text("Total index count %d", m_MeshManager->GetWorldIndicesCount());		

        Rbk::Im::Text("Stats per mesh :");
        for (std::shared_ptr<Mesh> mesh : *m_MeshManager->GetWorldMeshes()) {
            Rbk::Im::Text("\tMesh named %s with %d instances", mesh.get()->name.c_str(), mesh.get()->ubos.size());
        }
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
        ImGui::SliderFloat("Ambiant light", &Rbk::VulkanAdapter::s_AmbiantLight, 0.0f, 1.0f, "%.3f");
        ImGui::Separator();
        ImGui::SliderFloat("Fog density", &Rbk::VulkanAdapter::s_FogDensity, 0.0f, 1.0f, "%.3f");
        ImGui::Separator();
        ImGui::ColorEdit3("Fog color", Rbk::VulkanAdapter::s_FogColor);

        ImGui::Checkbox("Show ImGui demo", &m_ShowDemo);
        
        m_Adapter->SetWireFrameMode(m_WireframeModeOn);
        m_Adapter->MakeSpin(m_MakeSpin);
        m_Window->SetVSync(m_VSync);

        if (m_ShowDemo) {
            ImGui::ShowDemoWindow();
        }
    }

    void VulkanLayer::AddRenderAdapter(VulkanAdapter* renderAdapter)
    {
        m_Adapter = renderAdapter;
    }
}
