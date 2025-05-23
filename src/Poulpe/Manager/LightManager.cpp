module;
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

module Poulpe.Managers;

import Poulpe.Core.PlpTypedef;

namespace Poulpe
{
  LightManager::LightManager()
  {
    _sun.color = glm::vec3(1.f, 1.f, 1.f);
    _sun.position = glm::vec3(0.f, 200.f, 0.f);
    _sun.direction =  glm::vec3(-0.2f, -1.0f, -0.3f);
    //ambient diffuse specular
    _sun.ads = glm::vec3(10.f, 0.5f, 1.f);
    _sun.clq = glm::vec3(0.0f);

    _sun.view = glm::lookAt(
      glm::vec3(0.1f, 40.0f, 0.1f),
      glm::vec3(0.0f, 0.0f, 0.0f),
      glm::vec3(0.0f, 1.0f, 0.0f));

    float near_plane = 1.f, far_plane = 7.5f;

    //auto projection = glm::perspective(glm::radians(45.f),
    //    static_cast<float>(2560) / static_cast<float>(1440),
    //    near_plane, far_plane);

    auto projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

    //projection[1][1] *= -1;

    _sun.projection = projection;
    _sun.light_space_matrix = _sun.projection * _sun.view;

    Light light;
    light.color = glm::vec3(255.f/255.f, 100.f/255.f, 0.f);
    light.position = glm::vec3(0.0f, 20.f, 0.0f);
    light.direction = glm::vec3(-0.1f, -1.0, 0.0);
    light.ads = glm::vec3(10.0f, 30.0f, 40.0f);
    light.clq = glm::vec3(1.0f, 0.7f, 1.8f);

    Light light2;
    light2.color = glm::vec3(1.0f);
    //light2.position = glm::vec3(-1.2f, 0.1f, 0.4f);
    light2.position = glm::vec3(4.f, 25.0f, -1.5f);
    light2.direction = glm::vec3(-0.1f, -1.0, 0.0);
    light2.ads = glm::vec3(1.2f, 1.2f, 1.4f);
    light2.clq = glm::vec3(1.0f, 0.7f, 1.8f);

    _points.emplace_back(light2);
    _points.emplace_back(light);

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
    light3.light_space_matrix = light3.view * light3.projection;

    _spots.emplace_back(light3);
  }

  void LightManager::animateAmbientLight(double const delta_time)
  {
    auto& light1 = _points.at(0);
    light1.position.x += static_cast<float>(cos(glm::radians(delta_time * 360.0)));
    light1.position.z += static_cast<float>(sin(glm::radians(delta_time * 360.0)));
    light1.color *= delta_time;
  }

  void LightManager::animateSunLight(double const delta_time)
  {
    _sun.position.x += static_cast<float>(cos(glm::radians(delta_time * 360.0)));
    _sun.direction.x += static_cast<float>(cos(glm::radians(delta_time * 360.0)));
    _sun.position.z += static_cast<float>(sin(glm::radians(delta_time * 360.0)));
    _sun.direction.z += static_cast<float>(sin(glm::radians(delta_time * 360.0)));

    _sun.view = glm::lookAt(
      _sun.position,
      _sun.position + _sun.direction,
      glm::vec3(0.0f, 1.0f,  0.0f));

    float near_plane = 1.f, far_plane = 7.5f;
    _sun.projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    _sun.projection[1][1] *= -1;

    _sun.light_space_matrix = _sun.projection * _sun.view;
  }
}
