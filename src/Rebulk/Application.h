#pragma once

#include "rebulkpch.h"
#include "Manager/RenderManager.h"
#include "Rebulk/GUI/Window.h"

namespace Rbk 
{
	class Application
	{

	public:
		Application();
		~Application();

		inline static Application* Get() { return s_Instance; };
		void Run();

	private:
		static Application* s_Instance;
		std::shared_ptr<Rbk::Window>window = nullptr;
		std::shared_ptr<Rbk::RenderManager>renderManager = nullptr;
		std::shared_ptr<Rbk::IRendererAdapter>rendererAdapter = nullptr;
		bool m_Running = true;
	};
}