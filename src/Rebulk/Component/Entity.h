#pragma once

namespace Rbk
{
    class IVisitor;

    class Entity : public std::enable_shared_from_this<Entity>
    {
    public:
        struct BBox
        {
            glm::mat4 position;
            glm::vec3 center;
            glm::vec3 size;
        };

    public:
        virtual void Draw();
        virtual void Accept(std::shared_ptr<IVisitor> visitor);
        virtual ~Entity();
        
        inline const std::string& GetName() const { return m_Name; }
        void SetName(const std::string& name) { m_Name = name; }
        bool HasPushConstants() { return m_HasPushContants; }
        void SetHasPushConstants(bool has = true) { m_HasPushContants = has; }
        void SetVisible(bool visible) { m_IsVisible = visible; }
        bool IsVisible() { return m_IsVisible; }
        uint32_t GetNextSpriteIndex();
        void SetSpritesCount(uint32_t count) { m_SpritesCount = count; }
        bool IsHit(glm::vec3 point);
        void AddBBox(BBox bbox) { m_BoundingBox.emplace_back(bbox); }
        std::vector<BBox> GetBBox() { return m_BoundingBox; }

    private:
        std::string m_Name;
        bool m_HasPushContants = false;
        bool m_IsVisible = true;
        uint32_t m_SpritesCount = 0;
        uint32_t m_SpritesIndex = 0;
        std::vector<BBox> m_BoundingBox;
    };
}
