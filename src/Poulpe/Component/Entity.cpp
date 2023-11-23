#include "Entity.hpp"

#include <chrono>
#include <random>

namespace Poulpe
{
    Entity::Entity()
    {
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch()
        ).count();

        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dis(0, std::numeric_limits<uint64_t>::max());
        uint64_t random_number = dis(gen);
    
        m_ID = std::to_string(millis) + std::to_string(random_number);
    }

    void Entity::draw()
    {

    }

    Entity::~Entity()
    {

    }

    void Entity::accept(IVisitor* visitor)
    {
        visitor->visit(this);
    }

    uint32_t Entity::getNextSpriteIndex()
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

    bool Entity::isHit(glm::vec3 point)
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
