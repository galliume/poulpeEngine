#include "LightManager.hpp"

namespace Poulpe
{
    LightManager::LightManager()
    {
        m_AmbientLight.color = glm::vec3(1.0f);
        m_AmbientLight.direction = glm::vec3(-1.5f, 4.0f, -0.0f);
        m_AmbientLight.position = glm::vec3(-1.0f, 4.0f, 0.0f);
        //ambient diffuse specular
        m_AmbientLight.ads = glm::vec3(0.1f, 0.1f, 0.1f);
        m_AmbientLight.clq = glm::vec3(0.0f);

        Light light;
        light.color = glm::vec3(1.0);
        light.position = glm::vec3(-1.5f, 2.0f, 0.0f);
        light.ads = glm::vec3(0.1f, 0.2f, 0.4f);
        light.clq = glm::vec3(1.0f, 0.25f, 0.44f);

        Light light2;
        light2.color = glm::vec3(0.92f, 0.53f, 0.2f);
        light2.position = glm::vec3(1.0f, 0.2f, 0.0f);
        light2.ads = glm::vec3(0.5f, 0.5f, 1.f);
        light2.clq = glm::vec3(1.0f, 0.7f, 1.8f);

        m_PointLights.emplace_back(light);
        m_PointLights.emplace_back(light2);

        Light light3;
        light3.color = glm::vec3(0.2f, 0.53f, 0.92f);
        light3.position = glm::vec3(-1.5f, 0.2f, -0.5f);
        light3.direction = glm::vec3(0.0f);
        light3.ads = glm::vec3(0.5f, 0.8f, 1.0f);
        light3.clq = glm::vec3(1.0f, 0.7f, 1.8f);
        light3.coB = glm::vec3( glm::cos(glm::radians(8.0f)), glm::cos(glm::radians(9.0f)), 0.0f);

        m_SpotLights.emplace_back(light3);
    }
}
