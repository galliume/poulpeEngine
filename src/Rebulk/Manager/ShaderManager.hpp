#pragma once
#include "IShaderManager.hpp"
#include "Rebulk/Renderer/Adapter/IRendererAdapter.hpp"

namespace Rbk
{
    class ShaderManager : IShaderManager
    {
    public:
        explicit ShaderManager();
        ~ShaderManager();

        virtual void AddShader(const std::string& name, const std::string& vertPath, const std::string& fragPath) override;
        virtual std::function<void()> Load(nlohmann::json config) override;
        virtual inline const std::shared_ptr<VulkanShaders> GetShaders() const override { return m_Shaders; };

        void AddRenderer(std::shared_ptr<IRendererAdapter> renderer) { m_Renderer = renderer; };
        void Clear();
        bool IsLoadingDone() { return m_LoadingDone.load(); }

    private:
        std::shared_ptr<VulkanShaders> m_Shaders = nullptr;
        std::shared_ptr<IRendererAdapter> m_Renderer = nullptr;
        nlohmann::json m_Config;
        std::atomic_bool m_LoadingDone = false;
    };
}