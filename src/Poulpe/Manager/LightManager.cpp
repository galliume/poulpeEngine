#include "LightManager.hpp"

namespace Poulpe
{
    LightManager::LightManager()
    {
        m_AmbientLight.color = glm::vec3(1.0);
        m_AmbientLight.direction = glm::vec3(0.0, 4.0, 0.0);
        m_AmbientLight.ambient = glm::vec3(0.4);
        m_AmbientLight.diffuse = glm::vec3(0.7);
        m_AmbientLight.specular = glm::vec3(1.0f);
    }
}
