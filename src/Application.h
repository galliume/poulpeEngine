#pragma once

#include "rebulkpch.h"
#include "Renderer/Vulkan/VulkanRenderer.h"
#include "Renderer/Mesh.h"
#include "Core/TinyObjLoader.h"

namespace Rbk 
{
	class Application
	{

	public:
		Application();
		~Application();

		inline static Application& Get() { return *s_Instance; };
		void Run();

	private:
		static Application* s_Instance;
		bool m_Running = true;
	};
}