#pragma once
#include "Rebulk/Renderer/Adapter/IRendererAdapter.h"
#include "TextureManager.h"
#include "MeshManager.h"
#include "Rebulk/Renderer/Mesh.h"
#include "Rebulk/Component/Camera.h"

namespace Rbk
{
    class RenderManager
    {
    public:
        RenderManager(GLFWwindow* window, IRendererAdapter* renderer, TextureManager* textureManager, MeshManager* meshManager, ShaderManager* shaderManager);
        ~RenderManager();

        void Init();
        void AddCamera(Camera* camera);
        void PrepareDraw();
        void Draw();

        inline static RenderManager& Get() { return *s_Instance; };
        inline IRendererAdapter* Adp() { return m_Renderer; };

    private:
        static RenderManager* s_Instance;
        IRendererAdapter* m_Renderer = nullptr;
        GLFWwindow* m_Window;
        TextureManager* m_TextureManager;
        MeshManager* m_MeshManager;
        ShaderManager* m_ShaderManager;
    };
}