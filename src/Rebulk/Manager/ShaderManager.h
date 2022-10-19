#pragma once
#include "IShaderManager.h"

namespace Rbk
{
    class ShaderManager : IShaderManager
    {
    public:
        explicit ShaderManager();

        virtual void AddShader(const std::string& name, const std::string& vertPath, const std::string& fragPath) override;
        virtual std::future<void> Load(nlohmann::json config) override;
        virtual inline const std::shared_ptr<VulkanShaders> GetShaders() const override { return m_Shaders; };

        void AddRenderer(std::shared_ptr<IRendererAdapter> renderer) { m_Renderer = renderer; };

    private:
        std::shared_ptr<VulkanShaders> m_Shaders = nullptr;
        std::shared_ptr<IRendererAdapter> m_Renderer = nullptr;
    };
}
