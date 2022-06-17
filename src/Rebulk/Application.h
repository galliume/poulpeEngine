#pragma once

#include "Manager/RenderManager.h"
#include "Manager/InputManager.h"
#include "Manager/TextureManager.h"
#include "Manager/EntityManager.h"
#include "Renderer/Adapter/VulkanAdapter.h"

#include "GUI/LayerManager.h"
#include "GUI/Layer/VulkanLayer.h"
#include "GUI/ImGui/Im.h"
#include "GUI/Window.h"

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

        static int s_UnlockedFPS;

    private:
        static Application* s_Instance;
        std::shared_ptr<Rbk::Window> m_Window;
        std::shared_ptr<Rbk::RenderManager> m_RenderManager;
        std::shared_ptr<Rbk::Camera> m_Camera;
        std::shared_ptr<Rbk::InputManager> m_InputManager;
        std::shared_ptr<Rbk::VulkanAdapter> m_RendererAdapter;
        std::shared_ptr<Rbk::LayerManager> m_LayerManager;
        std::shared_ptr<Rbk::TextureManager> m_TextureManager;
        std::shared_ptr<Rbk::EntityManager> m_EntityManager;
        std::shared_ptr<Rbk::ShaderManager> m_ShaderManager;
        std::shared_ptr<Rbk::Im> m_VImGui;
    };
}