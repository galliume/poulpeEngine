#include "Entity.hpp"
#include "Poulpe/Core/IVisitor.hpp"

namespace Poulpe
{
    void Entity::Draw()
    {

    }

    void Entity::Accept(std::shared_ptr<IVisitor> visitor)
    {
        visitor->Visit(shared_from_this());
    }

    uint32_t Entity::GetNextSpriteIndex()
    {
        uint32_t nextIndex = 0;

        if (m_SpritesIndex > 0 && m_SpritesIndex < m_SpritesCount) {
            nextIndex = m_SpritesIndex;
            m_SpritesIndex += 1;
        } else if (m_SpritesIndex == m_SpritesCount  || 0 == m_SpritesIndex) {
            m_SpritesIndex = 1;
        }

        return nextIndex;
    }

    bool Entity::IsHit(glm::vec3 point)
    {
        //PLP_DEBUG(
        //   "BBOX maxX:{} minX{} maxY:{} minY:{} maxZ:{} minZ{}",
        //   GetBBox()->maxX,
        //   GetBBox()->minX,
        //   GetBBox()->maxY,
        //   GetBBox()->minY,
        //   GetBBox()->maxZ,
        //   GetBBox()->minZ
        //);
        // Oriented BBox intersection: http://www.opengl-tutorial.org/miscellaneous/clicking-on-objects/picking-with-custom-ray-obb-function/
        // 
        // 
        //glm::vec3 rayNds = glm::vec3(x, y, z);
        //glm::vec4 rayClip = glm::vec4(rayNds.x, rayNds.y, -1.0, 1.0);
        //glm::vec4 rayEye = glm::inverse(GetPerspective()) * rayClip;
        //rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0, 0.0);

        //glm::vec4 tmp = (glm::inverse(GetCamera()->GetView()) * rayEye);
        //glm::vec3 rayWor = glm::vec3(tmp.x, tmp.y, tmp.z);
        //m_RayPick = glm::normalize(rayWor);

        PLP_DEBUG("point x:{} y{} z:{}", point.x, point.y, point.z);

        //return (
        //    point.x >= GetBBox()->minX &&
        //    point.x <= GetBBox()->maxX &&
        //    point.y >= GetBBox()->minY &&
        //    point.y <= GetBBox()->maxY &&
        //    point.z >= GetBBox()->minZ &&
        //    point.z <= GetBBox()->maxZ
        //);

        return false;
    }
}