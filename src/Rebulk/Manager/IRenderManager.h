#pragma once
#include "Rebulk/Renderer/Adapter/IRendererAdapter.h"
#include "IConfigManager.h"
#include "IInputManager.h"
#include "IAudioManager.h"
#include "ITextureManager.h"
#include "IEntityManager.h"
#include "ISpriteAnimationManager.h"
#include "IShaderManager.h"
#include "Rebulk/Component/Mesh.h"
#include "Rebulk/Component/Camera.h"
#include "Rebulk/GUI/Window.h"
#include "Rebulk/GUI/ILayerManager.h"

namespace Rbk
{
    class IRenderManager
    {
    public:
        IRenderManager() = default;
        ~IRenderManager() = default;

        virtual void Init() = 0;
        virtual void Draw() = 0;
        virtual void SetDeltatime(float deltaTime) = 0;
        virtual inline std::shared_ptr<IRendererAdapter> Adp() = 0;
        virtual bool IsLoaded() = 0;
        virtual void SetIsLoaded(bool loaded = true) = 0;
        virtual std::shared_ptr<Window> GetWindow() = 0;
        virtual std::shared_ptr<Rbk::Camera> GetCamera() = 0;
        virtual std::shared_ptr<Rbk::IInputManager> GetInputManager() = 0;
        virtual std::shared_ptr<Rbk::IRendererAdapter> GetRendererAdapter() = 0;
        virtual std::shared_ptr<Rbk::ILayerManager> GetLayerManager() = 0;
        virtual std::shared_ptr<Rbk::ITextureManager> GetTextureManager() = 0;
        virtual std::shared_ptr<Rbk::ISpriteAnimationManager> GetSpriteAnimationManager() = 0;
        virtual std::shared_ptr<Rbk::IEntityManager> GetEntityManager() = 0;
        virtual std::shared_ptr<Rbk::IShaderManager> GetShaderManager() = 0;
        virtual std::shared_ptr<Rbk::IAudioManager> GetAudioManager() = 0;
        virtual std::shared_ptr<Rbk::IConfigManager> GetConfigManager() = 0;
        virtual std::shared_ptr<Rbk::ILayer> GetVulkanLayer() = 0;
    };
}