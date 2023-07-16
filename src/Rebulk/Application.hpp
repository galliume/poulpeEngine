#pragma once

#include "Manager/IRenderManager.hpp"
#include "Manager/InputManager.hpp"
#include "Manager/TextureManager.hpp"
#include "Manager/SpriteAnimationManager.hpp"
#include "Manager/EntityManager.hpp"
#include "Manager/AudioManager.hpp"
#include "Manager/ConfigManager.hpp"
#include "Manager/ShaderManager.hpp"
#include "Manager/DestroyManager.hpp"
#include "Renderer/Adapter/VulkanAdapter.hpp"

#include "GUI/LayerManager.hpp"
#include "GUI/Layer/VulkanLayer.hpp"
#include "GUI/ImGui/Im.hpp"
#include "GUI/Window.hpp"

#include "Core/CommandQueue.hpp"

namespace Rbk 
{
    class Application
    {

    public:
        Application();

        inline static Application* Get() { return s_Instance; };
        void Init();
        void Run();

        static  std::atomic<int> s_UnlockedFPS;

    private:
        static Application* s_Instance;
        std::shared_ptr<Rbk::Window> m_Window;
        std::shared_ptr<Rbk::RenderManager> m_RenderManager;
        std::shared_ptr<Rbk::Im> m_VImGui;
        std::shared_ptr<Rbk::VulkanLayer> m_VulkanLayer;
        std::shared_ptr<Rbk::CommandQueue> m_CommandQueue;

        std::chrono::time_point<std::chrono::system_clock> m_StartRun;
    };
}