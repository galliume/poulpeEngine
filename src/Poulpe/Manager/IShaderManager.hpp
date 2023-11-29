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

        virtual void addShader(std::string const & name, std::string const & vertPath, std::string const & fragPath) = 0;
        virtual std::function<void()> load(nlohmann::json config) = 0;
        virtual inline VulkanShaders* getShaders() const = 0;
    };
}
