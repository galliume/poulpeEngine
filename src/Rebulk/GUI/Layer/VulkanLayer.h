#pragma once
#include "rebulkpch.h"
#include "Rebulk/GUI/ImGui/Im.h"
#include "ILayer.h"
#include "Rebulk/Renderer/Adapter/VulkanAdapter.h"

namespace Rbk 
{
	class VulkanLayer : public ILayer
	{
	public:
		VulkanLayer();
		virtual void Init() override;
		void Destroy();
		void DisplayFpsCounter(double timeStep);
		void DisplayAPI(VkPhysicalDeviceProperties devicesProps);
		void Render(double timeStep, VkPhysicalDeviceProperties devicesProps);
		void DisplayOptions();
		void AddRenderAdapter(VulkanAdapter* renderAdapter);

	private:
		bool m_WireframeModeOn = false;
		VulkanAdapter* m_Adapter;
	};
}