#pragma once
#include <volk.h>

namespace Rbk
{
    class Vertex;
    struct UniformBufferObject;

    struct Buffer {
        VkBuffer buffer;
        std::shared_ptr<VkDeviceMemory> memory;
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
        Buffer m_VertexBuffer = { nullptr, nullptr, 0, 0 };
        Buffer m_IndicesBuffer = { nullptr, nullptr, 0, 0 };
        uint32_t m_TextureIndex = 0;
    };
}
