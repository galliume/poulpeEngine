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
		Rbk::Im::Begin("Performances stats");
		Rbk::Im::Text("FPS : %f", 1/timeStep);
		Rbk::Im::Text("Frametime : %f", timeStep);
		Rbk::Im::End();
	}

	void VulkanLayer::DisplayAPI(VkPhysicalDeviceProperties devicesProps)
	{
		Rbk::Im::Begin("API");
		Rbk::Im::Text("API Version : %d", devicesProps.apiVersion);
		Rbk::Im::Text("Drivers version : %d", devicesProps.driverVersion);
		Rbk::Im::Text("Vendor id : %d", devicesProps.vendorID);
		Rbk::Im::Text("GPU : %s" , devicesProps.deviceName);
		Rbk::Im::End();
	}
}