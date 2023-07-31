#pragma once
#include <volk.h>
#include <future>
#include <nlohmann/json.hpp>

namespace Poulpe
{
    struct VulkanShaders
    {
        std::unordered_map<std::string, std::array<VkShaderModule, 2>> shaders;
    };

    class IShaderManager
    {
    public:
        IShaderManager() = default;
        ~IShaderManager() = default;

        virtual void AddShader(const std::string& name, const std::string& vertPath, const std::string& fragPath) = 0;
        virtual std::function<void()> Load(nlohmann::json config, std::condition_variable& cv) = 0;
        virtual inline const std::shared_ptr<VulkanShaders> GetShaders() const = 0;
    };
}
