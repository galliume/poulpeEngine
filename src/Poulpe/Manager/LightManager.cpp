#include "LightManager.hpp"

namespace Poulpe
{
    LightManager::LightManager()
    {
        m_AmbientLight.color = glm::vec3(1.0f);
        m_AmbientLight.direction = glm::vec3(-0.1f, -1.0, 0.0);
        m_AmbientLight.position = glm::vec3(-0.6f, 4.5f, 0.0f);
        //ambient diffuse specular
        m_AmbientLight.ads = glm::vec3(0.5f, 0.7f, 1.0f);
        m_AmbientLight.clq = glm::vec3(0.0f);

        float near_plane = 1.0f, far_plane = 7.5f;
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        glm::mat4 lightView = glm::lookAt(m_AmbientLight.direction,
          glm::vec3(0.0f, 0.0f, 0.0f),
          glm::vec3(0.0f, 1.0f, 0.0f));
        m_AmbientLight.lightSpaceMatrix = lightProjection * lightView;

        Light light;
        light.color = glm::vec3(1.0);
        light.position = glm::vec3(-1.5f, 2.0f, 0.0f);
        light.direction = glm::vec3(-0.1f, -1.0, 0.0);
        light.ads = glm::vec3(0.2f, 0.5f, 1.0f);
        light.clq = glm::vec3(1.0f, 0.25f, 0.44f);

        Light light2;
        light2.color = glm::vec3(0.89f, 0.35f, 0.13f);
        light2.position = glm::vec3(1.0f, 0.2f, 0.0f);
        light2.direction = glm::vec3(-0.1f, -1.0, 0.0);
        light2.ads = glm::vec3(0.7f, 0.9f, 1.0f);
        light2.clq = glm::vec3(1.0f, 0.7f, 1.8f);

        m_PointLights.emplace_back(light);
        m_PointLights.emplace_back(light2);

        Light light3;
        light3.color = glm::vec3(0.13f, 0.35f, 0.89f);
        light3.position = glm::vec3(-0.9f, 1.5f, -0.2f);
        light3.direction = glm::vec3(-0.1f, -1.0, 0.0);
        light3.ads = glm::vec3(2.5f, 2.8f, 3.0f);
        light3.clq = glm::vec3(1.0f, 0.9f, 0.032f);
        light3.coB = glm::vec3( glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(15.0f)), 0.0f);

        m_SpotLights.emplace_back(light3);
    }
}
