#pragma once
#include "Rebulk/Component/Mesh.h"
#include "Rebulk/Component/Camera.h"
#include "Rebulk/Manager/TextureManager.h"
#include "Rebulk/Manager/EntityManager.h"
#include "Rebulk/Manager/ShaderManager.h"

namespace Rbk
{
    class IRendererAdapter
    {
    public:
        virtual void Init() = 0;
        virtual void AddTextureManager(std::shared_ptr<TextureManager> textureManager) = 0;
        virtual void AddEntityManager(std::shared_ptr<EntityManager> EntityManager) = 0;
        virtual void AddShaderManager(std::shared_ptr<ShaderManager> shaderManager) = 0;
        virtual void AddCamera(std::shared_ptr<Camera> camera) = 0;
        virtual void Prepare() = 0;
        virtual void Draw() = 0;
        virtual void Destroy() = 0;
        virtual void SetDeltatime(float deltaTime) = 0;
    };
}