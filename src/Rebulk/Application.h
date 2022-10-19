#pragma once

#include "Manager/RenderManager.h"
#include "Manager/InputManager.h"
#include "Manager/TextureManager.h"
#include "Manager/SpriteAnimationManager.h"
#include "Manager/EntityManager.h"
#include "Manager/AudioManager.h"
#include "Manager/ConfigManager.h"
#include "Manager/ShaderManager.h"
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

        static int s_UnlockedFPS;

    private:
        static Application* s_Instance;
        std::shared_ptr<Rbk::Window> m_Window;
        std::shared_ptr<Rbk::RenderManager> m_RenderManager;
        std::shared_ptr<Rbk::Im> m_VImGui;
    };
}