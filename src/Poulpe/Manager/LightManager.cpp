#include "LightManager.hpp"

namespace Poulpe
{
    LightManager::LightManager()
    {
        _AmbientLight.color = glm::vec3(1.f);
        _AmbientLight.position = glm::vec3(-2.0f, 4.0f, -1.0f);
        _AmbientLight.direction = glm::vec3(0.1f, -1.0, 0.0);
        //ambient diffuse specular
        _AmbientLight.ads = glm::vec3(1.5f, 0.7f, 1.0f);
        _AmbientLight.clq = glm::vec3(0.0f);

        _AmbientLight.view = glm::lookAt(
          glm::vec3(-2.0f, 4.0f, -1.0f),
          glm::vec3( 0.0f, 0.0f,  0.0f),
          glm::vec3( 0.0f, 1.0f,  0.0f));

        float near_plane = 1.f, far_plane = 7.5f;
        
        //auto projection = glm::perspective(glm::radians(45.f),
        //    static_cast<float>(2560) / static_cast<float>(1440),
        //    near_plane, far_plane);

        auto projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        projection[1][1] *= -1;

        _AmbientLight.projection = projection;
        _AmbientLight.lightSpaceMatrix = _AmbientLight.projection * _AmbientLight.view;

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

        _PointLights.emplace_back(light);
        _PointLights.emplace_back(light2);

        Light light3;
        light3.color = glm::vec3(0.13f, 0.35f, 0.89f);
        light3.position = glm::vec3(3.0f, 5.0f, 0.f);
        light3.direction = glm::vec3(3.0f, 5.0f, 0.f);
        light3.ads = glm::vec3(2.5f, 2.8f, 3.0f);
        light3.clq = glm::vec3(1.0f, 0.9f, 0.032f);

        light3.coB = glm::vec3( glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(15.0f)), 0.0f);

        light3.view = glm::lookAt(
            light3.position,
            glm::vec3( 0.0f, 0.0f,  0.0f),
            glm::vec3( 0.0f, 1.0f,  0.0f));

        light3.projection = glm::perspective(glm::radians(45.0f), 2560.f / 1440.f, 1.f, 100.f);
        light3.projection[1][1] *= -1;
        light3.lightSpaceMatrix = light3.view * light3.projection;

        _SpotLights.emplace_back(light3);
    }

    void LightManager::animateAmbientLight(float deltaTime)
    {
        _AmbientLight.position.x += cos(glm::radians(deltaTime * 360.0f));
        _AmbientLight.direction.x += cos(glm::radians(deltaTime * 360.0f));
        _AmbientLight.position.z += sin(glm::radians(deltaTime * 360.0f));
        _AmbientLight.direction.z += sin(glm::radians(deltaTime * 360.0f));

        _AmbientLight.view = glm::lookAt(
            _AmbientLight.position,
            _AmbientLight.position + _AmbientLight.direction,
            glm::vec3(0.0f, 1.0f,  0.0f));

        float near_plane = 1.f, far_plane = 7.5f;
        _AmbientLight.projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        _AmbientLight.projection[1][1] *= -1;

        _AmbientLight.lightSpaceMatrix = _AmbientLight.projection * _AmbientLight.view;
    }
}
