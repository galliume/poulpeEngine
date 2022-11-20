#pragma once

#include "Manager/IRenderManager.h"
#include "Manager/InputManager.h"
#include "Manager/TextureManager.h"
#include "Manager/SpriteAnimationManager.h"
#include "Manager/EntityManager.h"
#include "Manager/AudioManager.h"
#include "Manager/ConfigManager.h"
#include "Manager/ShaderManager.h"
#include "Manager/DestroyManager.h"
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

        inline static Application* Get() { return s_Instance; };
        void Init();
        void Run();

        static  std::atomic<int> s_UnlockedFPS;

    private:
        void InitImGui();

    private:
        static Application* s_Instance;
        std::shared_ptr<Rbk::Window> m_Window;
        std::shared_ptr<Rbk::RenderManager> m_RenderManager;
        std::shared_ptr<Rbk::Im> m_VImGui;
        std::shared_ptr<Rbk::VulkanLayer> m_VulkanLayer;

        std::chrono::time_point<std::chrono::system_clock> m_StartRun;
    };
}