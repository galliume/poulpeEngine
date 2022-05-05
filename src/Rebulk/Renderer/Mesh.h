#pragma once
#include <volk.h>

namespace Rbk
{
    struct Texture
    {
        const char* name;
        VkImage image;
        VkDeviceMemory imageMemory;
        VkImageView imageView;
        VkSampler sampler;
        uint32_t mipLevels;
        uint32_t width;
        uint32_t height;
        uint32_t channels;
        VkImage colorImage;
        VkDeviceMemory colorImageMemory;
        VkImageView colorImageView;
        VkImage depthImage;
        VkDeviceMemory depthImageMemory;
        VkImageView depthImageView;
    };

    struct UniformBufferObject {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };

    struct CubeUniformBufferObject : UniformBufferObject {
        int index;
    };

    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;

        static VkVertexInputBindingDescription GetBindingDescription() {

            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions() {

            std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, texCoord);;

            return attributeDescriptions;
        }

        bool operator==(const Vertex& other) const
        {
            return pos == other.pos && color == other.color && texCoord == other.texCoord;
        }
    };

    struct Vertex2D
    {
        glm::vec2 pos;
        glm::vec3 color;

        static VkVertexInputBindingDescription GetBindingDescription() {

            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex2D);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions() {

            std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex2D, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex2D, color);

            return attributeDescriptions;
        }

        bool operator==(const Vertex2D& other) const
        {
            return pos == other.pos && color == other.color;
        }
    };

    struct Mesh
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        const char* texture;
        const char* name;
        std::vector<UniformBufferObject> ubos;
        std::pair<VkBuffer, VkDeviceMemory> vertexBuffer = { nullptr, nullptr };
        std::pair<VkBuffer, VkDeviceMemory> indicesBuffer = { nullptr, nullptr };
        std::vector<std::pair<VkBuffer, VkDeviceMemory>> uniformBuffers;
        std::vector<VkDescriptorSet> descriptorSets;
        VkPipelineLayout pipelineLayout;
        VkPipeline graphicsPipeline;
        VkPipelineCache pipelineCache = 0;
    };

    struct Mesh2D : Mesh
    {
        std::vector<Vertex2D> vertices;
    };
}

namespace std {
    template<> struct hash<Rbk::Vertex> {
        size_t operator()(Rbk::Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
    template<> struct hash<Rbk::Vertex2D> {
        size_t operator()(Rbk::Vertex2D const& vertex) const {
            return ((hash<glm::vec2>()(vertex.pos) ^
                (hash<glm::vec3>()(vertex.color) << 1)) >> 1);
        }
    };
}
