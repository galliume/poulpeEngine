#pragma once

#include "Core/CommandQueue.hpp"

#include "GUI/ImGui/Im.hpp"
#include "GUI/LayerManager.hpp"
#include "GUI/Layer/VulkanLayer.hpp"
#include "GUI/Window.hpp"

#include "Manager/AudioManager.hpp"
#include "Manager/ConfigManager.hpp"
#include "Manager/DestroyManager.hpp"
#include "Manager/EntityManager.hpp"
#include "Manager/InputManager.hpp"
#include "Manager/IRenderManager.hpp"
#include "Manager/ShaderManager.hpp"
#include "Manager/SpriteAnimationManager.hpp"
#include "Manager/TextureManager.hpp"

#include "Renderer/Adapter/VulkanAdapter.hpp"

namespace Poulpe
{
    class Application
    {

    public:
        Application();

        inline static Application* get() { return s_Instance; };
        void init();
        void run();

        static  std::atomic<int> s_UnlockedFPS;

    private:
        static Application* s_Instance;

        std::shared_ptr<Poulpe::Window> m_Window;
        std::shared_ptr<Poulpe::RenderManager> m_RenderManager;
        std::shared_ptr<Poulpe::VulkanLayer> m_VulkanLayer;

        std::chrono::time_point<std::chrono::system_clock> m_StartRun;
    };
}