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
		void AddTextureManager(TextureManager* textureManager) { m_TextureManager = textureManager; };
		void AddMeshManager(MeshManager* meshManager) { m_MeshManager = meshManager; };
		void AddShaderManager(ShaderManager* shaderManager) { m_ShaderManager = shaderManager; };

	private:
		bool m_WireframeModeOn = false;
		bool m_VSync = false;
		bool m_ShowDemo = false;
		bool m_MakeSpin = false;
		VulkanAdapter* m_Adapter;
		Window* m_Window;
		TextureManager* m_TextureManager;
		MeshManager* m_MeshManager;
		ShaderManager* m_ShaderManager;
	};
}