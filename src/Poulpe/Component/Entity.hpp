#pragma once

namespace Poulpe
{
    class IVisitor;
    class Mesh;

    class Entity : public std::enable_shared_from_this<Entity>
    {
    public:
        struct BBox
        {
            glm::mat4 position;
            glm::vec3 center;
            glm::vec3 size;
            std::shared_ptr<Mesh> mesh;
            float maxX;
            float minX;
            float maxY;
            float minY;
            float maxZ;
            float minZ;
        };

    public:
        virtual void Draw();
        virtual void Accept(std::shared_ptr<IVisitor> visitor);
        virtual ~Entity() = default;
        
        [[nodiscard]] inline const std::string &GetName() const {
            return m_Name;
        }
        void SetName(const std::string& name) { m_Name = name; }
        bool HasPushConstants() { return m_HasPushContants; }
        void SetHasPushConstants(bool has = true) { m_HasPushContants = has; }
        void SetVisible(bool visible) { m_IsVisible = visible; }
        bool IsVisible() { return m_IsVisible; }
        uint32_t GetNextSpriteIndex();
        void SetSpritesCount(uint32_t count) { m_SpritesCount = count; }
        bool IsHit(glm::vec3 point);
        void AddBBox(std::shared_ptr<BBox> bbox) { m_BoundingBox = bbox; }
        std::shared_ptr<BBox> GetBBox() { return m_BoundingBox; }

    private:
        std::string m_Name;
        bool m_HasPushContants = false;
        bool m_IsVisible = true;
        uint32_t m_SpritesCount = 0;
        uint32_t m_SpritesIndex = 0;
        std::shared_ptr<BBox> m_BoundingBox;
    };
}