#pragma once
#include "Poulpe/Renderer/Vulkan/DeviceMemory.hpp"

namespace Poulpe
{
    class Vertex;
    struct UniformBufferObject;

    struct Buffer {
        VkBuffer buffer;
        std::shared_ptr<DeviceMemory> memory;
        uint32_t offset;
        uint32_t size;
    };

    struct Data
    {
        std::string m_Name;
        std::string m_Texture;
        std::vector<Vertex> m_Vertices;
        std::vector<uint32_t> m_Indices;
        std::vector<UniformBufferObject> m_Ubos;
        std::vector<uint32_t> m_UbosOffset;
        Buffer m_VertexBuffer = { nullptr, nullptr, 0, 0 };
        Buffer m_IndicesBuffer = { nullptr, nullptr, 0, 0 };
        uint32_t m_TextureIndex = 0;
    };
}
