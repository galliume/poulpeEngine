#include "rebulkpch.h"
#include "VulkanLayer.h"
#include <inttypes.h>

namespace Rbk
{
    void VulkanLayer::Init()
    {
        //DisplayFpsCounter(timeStep);
        //DisplayAPI(renderer->GetDeviceProperties());
    }

    void VulkanLayer::Render(double timeStep, VkPhysicalDeviceProperties devicesProps)
    {
        ImGui::Begin("Performances stats");

        DisplayFpsCounter(timeStep);
        ImGui::Separator();
        DisplayAPI(devicesProps);
        ImGui::Separator();
        DisplayOptions();

        ImGui::End();
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
            Rbk::Im::Text("\tMesh named %s with %d instances", mesh.get()->name, mesh.get()->ubos.size());
        }
        ImGui::Separator();
        Rbk::Im::Text("Shader count %d", m_ShaderManager->GetShaders()->shaders.size());
        ImGui::Separator();
        Rbk::Im::Text("Texture count %d", m_TextureManager->GetTextures().size());
        Rbk::Im::Text("Loaded textures :");
        for (auto tex : m_TextureManager->GetTextures()) {
            Rbk::Im::Text("\t%s", tex.first);
        }
    }

    void VulkanLayer::DisplayOptions()
    {        
        ImGui::Checkbox("VSync", &m_VSync);
        ImGui::Checkbox("Wireframe", &m_WireframeModeOn);
        ImGui::Checkbox("Show ImGui demo", &m_ShowDemo);
        ImGui::Checkbox("Make spin", &m_MakeSpin);
        
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
