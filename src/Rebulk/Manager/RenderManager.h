#pragma once
#include "Rebulk/Renderer/Adapter/IRendererAdapter.h"
#include "TextureManager.h"
#include "MeshManager.h"
#include "Rebulk/Renderer/Mesh.h"
#include "Rebulk/Component/Camera.h"
#include "Rebulk/GUI/Window.h"

namespace Rbk
{
    class RenderManager
    {
    public:
        explicit RenderManager(
            std::shared_ptr<Window> window,
            std::shared_ptr<IRendererAdapter> renderer,
            std::shared_ptr<TextureManager> textureManager,
            std::shared_ptr<MeshManager> meshManager,
            std::shared_ptr<ShaderManager> shaderManager
        );
        ~RenderManager();

        void Init();
        void AddCamera(std::shared_ptr<Camera> camera);
        void PrepareDraw();
        void Draw();

        inline static RenderManager& Get() { return *s_Instance; };
        inline std::shared_ptr<IRendererAdapter> Adp() { return m_Renderer; };

    private:
        static RenderManager* s_Instance;
        std::shared_ptr<IRendererAdapter> m_Renderer = nullptr;
        std::shared_ptr<Window> m_Window;
        std::shared_ptr<TextureManager> m_TextureManager;
        std::shared_ptr<MeshManager> m_MeshManager;
        std::shared_ptr<ShaderManager> m_ShaderManager;
    };
}