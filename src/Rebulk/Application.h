#pragma once

#include "rebulkpch.h"

#include "Manager/RenderManager.h"
#include "Manager/InputManager.h"

#include "Renderer/Adapter/VulkanAdapter.h"

#include "GUI/LayerManager.h"
#include "GUI/Layer/VulkanLayer.h"
#include "GUI/ImGui/Im.h"
#include "Rebulk/GUI/Window.h"

namespace Rbk 
{
	class Application
	{

	public:
		Application();
		~Application();

		inline static Application* Get() { return s_Instance; };
		void Init();
		void Run();

	private:
		static Application* s_Instance;
		bool m_Running = true;
	};
}