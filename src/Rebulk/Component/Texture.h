#pragma once
#include <volk.h>

namespace Rbk
{
    class Texture
    {
    public:
        std::string name;
        VkImage image;
        VkDeviceMemory imageMemory;
        VkImageView imageView;
        VkSampler sampler;
        uint32_t mipLevels;
        uint32_t width;
        uint32_t height;
        uint32_t channels;
    };
}