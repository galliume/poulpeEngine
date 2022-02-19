#include "VulkanLayer.h"
#include <inttypes.h>

namespace Rbk
{
	VulkanLayer::VulkanLayer()
	{

	}

	void VulkanLayer::Init()
	{
		//DisplayFpsCounter(timeStep);
		//DisplayAPI(renderer->GetDeviceProperties());
	}

	void VulkanLayer::Render(double timeStep, VkPhysicalDeviceProperties devicesProps)
	{
		DisplayFpsCounter(timeStep);
		DisplayAPI(devicesProps);		
	}
	void VulkanLayer::Destroy()
	{
		Rbk::Im::Destroy();
	}

	void VulkanLayer::DisplayFpsCounter(double timeStep)
	{
		ImGui::Begin("Performances stats");	
		Rbk::Im::Text("FPS : %.2f", 1 / timeStep);
		Rbk::Im::Text("Frametime : %.2f ms", timeStep * 1000);	
		ImGui::End();
	}

	void VulkanLayer::DisplayAPI(VkPhysicalDeviceProperties devicesProps)
	{
		ImGui::Begin("Informations");
					
		Rbk::Im::Text("API Version : %d", devicesProps.apiVersion);
		Rbk::Im::Text("Drivers version : %d", devicesProps.driverVersion);
		Rbk::Im::Text("Vendor id : %d", devicesProps.vendorID);
		Rbk::Im::Text("GPU : %s", devicesProps.deviceName);
		ImGui::Separator();
		Rbk::Im::Text("Current frame %d", m_Adapter->Rdr()->GetCurrentFrame());
		ImGui::Separator();
		Rbk::Im::Text("Total mesh loaded %d", m_Adapter->GetMesh().count);
		Rbk::Im::Text("Total mesh instanced %d", m_Adapter->GetMesh().totalInstances);
		Rbk::Im::Text("Total vertex count %d", m_Adapter->GetMesh().mesh.vertices.size());
		Rbk::Im::Text("Total index count %d", m_Adapter->GetMesh().mesh.indices.size());
		Rbk::Im::Text("Max UBO buffer size by chunk %d", m_Adapter->GetMesh().maxUniformBufferRange);
		Rbk::Im::Text("UBO buffer size by chunk %d", m_Adapter->GetMesh().uniformBufferChunkSize);
		Rbk::Im::Text("Total UBO buffer %d", m_Adapter->GetMesh().uniformBuffersCount);

		Rbk::Im::Text("Stats per mesh :");
		for (auto item : m_Adapter->GetMesh().mesh.meshNames) {
			Rbk::Im::Text("\tMesh named %s with %d instances", item.first, item.second);
		}
		ImGui::Separator();
		Rbk::Im::Text("Shader count %d", m_Adapter->GetShaders().shaders.size());
		ImGui::Separator();
		Rbk::Im::Text("Texture count %d", m_TextureManager->GetTextures().size());
		Rbk::Im::Text("Loaded textures :");
		for (auto tex : m_TextureManager->GetTextures()) {
			Rbk::Im::Text("\t%s", tex.first);
		}

		ImGui::End();
	}

	void VulkanLayer::DisplayOptions()
	{
		ImGui::Begin("Rendering options");
		
		ImGui::Checkbox("VSync", &m_VSync);
		ImGui::Checkbox("Wireframe", &m_WireframeModeOn);
		ImGui::Checkbox("Show ImGui demo", &m_ShowDemo);
		ImGui::Checkbox("Make spin", &m_MakeSpin);

		ImGui::End();
		
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