#pragma once
#include "rebulkpch.h"
#include "Rebulk/GUI/ImGui/Im.h"
#include "Rebulk/Pattern/IObserver.h"
#include "ILayer.h"

namespace Rbk 
{
	class VulkanLayer : public ILayer, public IObserver
	{
	public:
		VulkanLayer();
		virtual void Init() override;
		void DisplayLogs();
		void Destroy();
		void Update(std::vector<std::string>& messages);
		void DisplayFpsCounter(double timeStep);
		void DisplayAPI(VkPhysicalDeviceProperties devicesProps);

	private:
		std::string m_Message;
		std::vector<std::string>m_Messages = {};
		int m_MaxMessages = 500;
	};
}