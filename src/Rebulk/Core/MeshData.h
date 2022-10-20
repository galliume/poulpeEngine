#pragma once
#include <volk.h>

namespace Rbk
{
    class Vertex;
    struct UniformBufferObject;

    struct Data
    {
        std::string m_Name;
        std::string m_Texture;
        std::vector<Vertex> m_Vertices;
        std::vector<uint32_t> m_Indices;
        std::vector<UniformBufferObject> m_Ubos;
        std::pair<VkBuffer, VkDeviceMemory> m_VertexBuffer = { nullptr, nullptr };
        std::pair<VkBuffer, VkDeviceMemory> m_IndicesBuffer = { nullptr, nullptr };
        uint32_t m_TextureIndex = 0;
    };
}
