#pragma once
#include "IShaderManager.hpp"
#include "Poulpe/Renderer/Adapter/IRendererAdapter.hpp"

namespace Poulpe
{
    class ShaderManager : IShaderManager
    {
    public:
        explicit ShaderManager();
        virtual ~ShaderManager() = default;

        virtual void AddShader(const std::string& name, const std::string& vertPath, const std::string& fragPath) override;
        virtual std::function<void()> Load(nlohmann::json config, std::condition_variable& cv) override;
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
