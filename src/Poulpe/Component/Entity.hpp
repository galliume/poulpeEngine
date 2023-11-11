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
            std::unique_ptr<Mesh> mesh;
            float maxX;
            float minX;
            float maxY;
            float minY;
            float maxZ;
            float minZ;
        };

    public:
        virtual void draw();
        virtual void accept(IVisitor* visitor);
        virtual ~Entity() = default;

        [[nodiscard]] inline const std::string getName() const { return m_Name; }
        void setName(std::string const & name) { m_Name = name; }
        bool hasPushConstants() { return m_HasPushContants; }
        void setHasPushConstants(bool has = true) { m_HasPushContants = has; }
        void setVisible(bool visible) { m_IsVisible = visible; }
        bool isVisible() { return m_IsVisible; }
        uint32_t getNextSpriteIndex();
        void setSpritesCount(uint32_t count) { m_SpritesCount = count; }
        bool isHit(glm::vec3 point);
        void addBBox(std::shared_ptr<BBox> bbox) { m_BoundingBox = std::move(bbox); }
        std::shared_ptr<BBox> getBBox() { return m_BoundingBox; }

    private:
        std::string m_Name;
        bool m_HasPushContants = false;
        bool m_IsVisible = true;
        uint32_t m_SpritesCount = 0;
        uint32_t m_SpritesIndex = 0;
        std::shared_ptr<BBox> m_BoundingBox;
    };
}
