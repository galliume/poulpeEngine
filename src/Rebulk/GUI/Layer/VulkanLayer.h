#pragma once
#include "Rebulk/GUI/ImGui/Im.h"
#include "ILayer.h"
#include "Rebulk/Renderer/Adapter/VulkanAdapter.h"
#include "Rebulk/GUI/Window.h"
#include "Rebulk/Application.h"

namespace Rbk
{
    class VulkanLayer : public ILayer
    {
    public:
        virtual void Init() override;
        void Destroy();
        void DisplayFpsCounter(double timeStep);
        void DisplayAPI(VkPhysicalDeviceProperties devicesProps);
        void Render(double timeStep, VkPhysicalDeviceProperties devicesProps);
        void DisplayOptions();
        void AddRenderAdapter(std::shared_ptr<VulkanAdapter> renderAdapter);
        void AddWindow(std::shared_ptr<Window> window) { m_Window = window; };
        void AddTextureManager(std::shared_ptr<TextureManager> textureManager) { m_TextureManager = textureManager; };
        void AddMeshManager(std::shared_ptr<MeshManager> meshManager) { m_MeshManager = meshManager; };
        void AddShaderManager(std::shared_ptr<ShaderManager> shaderManager) { m_ShaderManager = shaderManager; };

    private:
        bool m_WireframeModeOn = false;
        bool m_VSync = false;
        bool m_ShowDemo = false;
        std::shared_ptr<VulkanAdapter> m_Adapter;
        std::shared_ptr<Window> m_Window;
        std::shared_ptr<TextureManager> m_TextureManager;
        std::shared_ptr<MeshManager> m_MeshManager;
        std::shared_ptr<ShaderManager> m_ShaderManager;
    };
}
