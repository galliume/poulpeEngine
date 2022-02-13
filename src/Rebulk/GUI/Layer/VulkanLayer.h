#pragma once
#include "rebulkpch.h"
#include "Rebulk/GUI/ImGui/Im.h"
#include "ILayer.h"
#include "Rebulk/Renderer/Adapter/VulkanAdapter.h"
#include "Rebulk/GUI/Window.h"

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
		void AddWindow(Window* window) { m_Window = window; };
	private:
		bool m_WireframeModeOn = false;
		bool m_VSync = false;
		bool m_ShowDemo = false;
		VulkanAdapter* m_Adapter;
		Window* m_Window;
	};
}