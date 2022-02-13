#include "VulkanLayer.h"
#include <inttypes.h>

namespace Rbk
{
	VulkanLayer::VulkanLayer()
	{
		m_WireframeModeOn = false;
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
		Rbk::Im::Text("FPS : %f", 1 / timeStep);
		Rbk::Im::Text("Frametime : %f", timeStep);	
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
		Rbk::Im::Text("Mesh count %d", m_Adapter->GetMesh().count);
		Rbk::Im::Text("Vertex count %d", m_Adapter->GetMesh().mesh.vertices.size());
		Rbk::Im::Text("Index count %d", m_Adapter->GetMesh().mesh.indices.size());
		ImGui::Separator();
		Rbk::Im::Text("Shader count %d", m_Adapter->GetShaders().shaders.size());
		ImGui::Separator();
		Rbk::Im::Text("Texture count %d", m_Adapter->GetTextures().size());
		Rbk::Im::Text("Loaded textures :");
		for (auto tex : m_Adapter->GetTextures()) {
			Rbk::Im::Text("\t%s", tex.first);
		}

		ImGui::End();
	}

	void VulkanLayer::DisplayOptions()
	{
		ImGui::Begin("Rendering options");
		
		ImGui::TableNextColumn(); ImGui::Checkbox("Wireframe", &m_WireframeModeOn);

		ImGui::End();
		
		m_Adapter->SetWireFrameMode(m_WireframeModeOn);
	}

	void VulkanLayer::AddRenderAdapter(VulkanAdapter* renderAdapter)
	{
		m_Adapter = renderAdapter;
	}
}