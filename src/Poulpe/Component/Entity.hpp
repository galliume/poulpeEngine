#pragma once

#include "Poulpe/Core/Buffer.hpp"
#include "Poulpe/Core/TinyObjLoader.hpp"
#include "Poulpe/Core/IVisitor.hpp"

#include "Poulpe/Renderer/Vulkan/DeviceMemory.hpp"

#include "Texture.hpp"

#include "Vertex.hpp"
#include "Vertex2D.hpp"

namespace Poulpe
{
    class Entity : public std::enable_shared_from_this<Entity>
    {

    public:
        Entity();
        virtual ~Entity() = 0;

        struct Buffer {
            VkBuffer buffer;
            DeviceMemory* memory;
            uint32_t offset;
            uint32_t size;
        };

        struct Data {
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

        void draw();
        void accept(IVisitor* visitor);
        
        uint32_t getNextSpriteIndex();
        Data* getData() { return & m_Data; }
        [[nodiscard]] inline const std::string getName() const { return m_Name; }
        inline std::vector<Buffer>* getUniformBuffers() { return & m_UniformBuffers; }
        std::string const GetID() const { return m_ID; }

        void setName(std::string const & name) { m_Name = name; }
        void setHasPushConstants(bool has = true) { m_HasPushContants = has; }
        void setVisible(bool visible) { m_IsVisible = visible; }
        void setSpritesCount(uint32_t count) { m_SpritesCount = count; }
        void setData(Data data) { m_Data = std::move(data); }

        bool hasPushConstants() { return m_HasPushContants; }
        
        bool isVisible() { return m_IsVisible; }
        bool isHit(glm::vec3 point);

        virtual bool isDirty() = 0;

    protected:
        Data m_Data;

    private:
        std::string m_ID;

        std::string m_Name;
        bool m_HasPushContants = false;
        bool m_IsVisible = true;
        uint32_t m_SpritesCount = 0;
        uint32_t m_SpritesIndex = 0;

        std::vector<Buffer> m_UniformBuffers;
    };
}
