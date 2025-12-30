module;


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>


module Engine.Managers.LightManager;

import std;

import Engine.Core.Logger;
import Engine.Core.PlpTypedef;

import Engine.Managers.ConfigManagerLocator;

namespace Poulpe
{
  LightManager::LightManager()
  {
    _sun.color = glm::vec3(0.5, 0.7, 1.0);//moonish color ?
    _sun.position = glm::vec3(0.f, 500.f, 0.f);
    _sun.direction =  glm::vec3(1.0f, -1.0f, 1.0f);
    //ambient diffuse specular
    _sun.ads = glm::vec3(10.f, 0.5f, 1.f);
    _sun.clq = glm::vec3(0.0f);

    auto const& appConfig { ConfigManagerLocator::get()->appConfig()["resolution"] };
    float const app_width { static_cast<float>(appConfig["width"].get<uint16_t>()) };
    float const app_height { static_cast<float>(appConfig["height"].get<uint16_t>()) };

    //float aspect = 1600.f / 900.f;
    float fov = glm::radians(90.0f);
    glm::mat4 shadow_cubemap_projection = glm::perspective(fov, 1.0f, 1.0f, 50.f);
    shadow_cubemap_projection[1][1] *= -1;

    Light light;
    light.color = glm::vec3(1.0, 0.6, 0.1);
    light.position = glm::vec3(0.0f, 50.f, 0.0f);
    light.direction = glm::vec3(-0.1f, -1.0f, 0.0f);
    light.ads = glm::vec3(10.0f, 30.0f, 40.0f);
    light.clq = glm::vec3(1.0f, 0.7f, 1.8f);

    light.projection = shadow_cubemap_projection;//glm::ortho(-10.0f, 10.0f, 10.0f, -10.0f, near_plane, far_plane);
    light.light_space_matrix_right = light.projection * glm::lookAt(
      light.position,
      light.position + glm::vec3(1.0f, 0.0f, 0.0f),
      glm::vec3(0.0f, -1.0f, 0.0f));

    light.light_space_matrix_left = light.projection * glm::lookAt(
      light.position,
      light.position + glm::vec3(-1.0f, 0.0f, 0.0f),
      glm::vec3(0.0f, -1.0f, 0.0f));

    light.light_space_matrix_top = light.projection * glm::lookAt(
      light.position,
      light.position + glm::vec3(0.0f, 1.0f, 0.0f),
      glm::vec3(0.0f, 0.0f, 1.0f));

    light.light_space_matrix_bottom = light.projection * glm::lookAt(
      light.position,
      light.position + glm::vec3(0.0f, -1.0f, 0.0f),
      glm::vec3(0.0f, 0.0f, -1.0f));

    light.light_space_matrix = light.projection * glm::lookAt(
      light.position,
      light.position + glm::vec3(0.0f, 0.0f, 1.0f),
      glm::vec3(0.0f, -1.0f, 0.0f)); //used as light_space_matrix_front

    light.light_space_matrix_back = light.projection * glm::lookAt(
      light.position,
      light.position + glm::vec3(0.0f, 0.0f, -1.0f),
      glm::vec3(0.0f, -1.0f, 0.0f));

    light.view = light.light_space_matrix;

    Light light2;
    light2.color = glm::vec3(1.0f);
    //light2.position = glm::vec3(-1.2f, 0.1f, 0.4f);
    light2.position = glm::vec3(5.f, 25.0f, 3.f);
    light2.direction = glm::vec3(0.0f, -1.0, 0.0);
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
      glm::vec3( 0.0f, 0.0f, 0.0f),
      glm::vec3( 0.0f, 0.0f, -1.0f));

    light3.projection = glm::perspective(glm::radians(45.0f), app_width / app_height, 1.f, 100.f);
    //light3.projection[1][1] *= -1;
    light3.light_space_matrix = light3.view * light3.projection;

    _spots.emplace_back(light3);
  }

  CSM LightManager::getLightSpaceMatrix(
    float const near_plane,
    float const far_plane,
    glm::mat4 const & M_camera,
    glm::mat4 const & projection)
  {
    auto const& appConfig { ConfigManagerLocator::get()->appConfig()["shadow_resolution"] };
    auto const shadow_resolution { static_cast<float>(appConfig["width"].get<uint32_t>()) };

    auto const g { projection[1][1] };
    auto const s { g / projection[0][0] };

    auto const M_camera_inv { glm::inverse(M_camera) };

    auto const near_h { near_plane / g };
    auto const near_w { near_h * s };
    auto const near_v0 { glm::vec3(near_w, near_h, -near_plane) };
    auto const near_v1 { glm::vec3(near_w, -near_h, -near_plane) };
    auto const near_v2 { glm::vec3(-near_w, -near_h, -near_plane) };
    auto const near_v3 { glm::vec3(-near_w, near_h, -near_plane) };

    auto const far_h { far_plane / g };
    auto const far_w { far_h * s };
    auto const far_v0 { glm::vec3(far_w, far_h, -far_plane) };
    auto const far_v1 { glm::vec3(far_w, -far_h, -far_plane) };
    auto const far_v2 { glm::vec3(-far_w, -far_h, -far_plane) };
    auto const far_v3 { glm::vec3(-far_w, far_h, -far_plane) };

    std::vector<glm::vec4> cascade_frustum {
      M_camera_inv * glm::vec4(near_v0, 1.0f),
      M_camera_inv * glm::vec4(near_v1, 1.0f),
      M_camera_inv * glm::vec4(near_v2, 1.0f),
      M_camera_inv * glm::vec4(near_v3, 1.0f),
      M_camera_inv * glm::vec4(far_v0, 1.0f),
      M_camera_inv * glm::vec4(far_v1, 1.0f),
      M_camera_inv * glm::vec4(far_v2, 1.0f),
      M_camera_inv * glm::vec4(far_v3, 1.0f)
    };

    for (auto& v : cascade_frustum) {
      v /= v.w;
    }

    auto center { std::accumulate(cascade_frustum.begin(), cascade_frustum.end(), glm::vec4(0.0f)) };
    center /= cascade_frustum.size();

    auto const M_light { glm::lookAt(
        glm::vec3(center) - glm::normalize(_sun.direction),
        glm::vec3(center),
        glm::vec3(0.0f, 1.0f, 0.0f)) };

    auto min_x { std::numeric_limits<float>::max() };
    auto max_x { std::numeric_limits<float>::lowest() };
    auto min_y { std::numeric_limits<float>::max() };
    auto max_y { std::numeric_limits<float>::lowest() };
    auto min_z { std::numeric_limits<float>::max() };
    auto max_z { std::numeric_limits<float>::lowest() };

    for (auto const& v : cascade_frustum) {
      auto const Lv { M_light * v };
      min_x = std::min(min_x, Lv.x);
      max_x = std::max(max_x, Lv.x);
      min_y = std::min(min_y, Lv.y);
      max_y = std::max(max_y, Lv.y);
      min_z = std::min(min_z, Lv.z);
      max_z = std::max(max_z, Lv.z);
    }

    auto const d {
      static_cast<std::uint32_t>(
      std::max(
        std::ceil(glm::length(cascade_frustum[0] - cascade_frustum[6])),
        std::ceil(glm::length(cascade_frustum[4] - cascade_frustum[6])))) };

    auto const T { d / shadow_resolution };

    auto const snapped_world_center {
      glm::vec3(
        std::floorf((max_x + min_x) / (2.0f * T)) * T,
        std::floorf((max_y + min_y) / (2.0f * T)) * T,
        (min_z + max_z) * 0.5f)
    };

    glm::mat3 const R { M_light };
    glm::mat4 m_cascade { glm::transpose(R) };
    m_cascade[3] = glm::vec4(-snapped_world_center, 1.0f);

    // glm::mat4 const m_cascade { glm::lookAt(
    //     snapped_world_center - glm::normalize(_sun.direction) * 100.0f,
    //     snapped_world_center,
    //     glm::vec3(0.0f, 1.0f, 0.0f)) };

    min_z = std::numeric_limits<float>::max();
    max_z = std::numeric_limits<float>::lowest();

    for (auto const& v : cascade_frustum) {
        float z = (m_cascade * v).z;
        min_z = std::min(min_z, z);
        max_z = std::max(max_z, z);
    }

    min_z -= 1000.0f;
    max_z += 1000.0f;

    auto const df {static_cast<float>(d) * 0.5f };
    auto const z_range { max_z - min_z };

    glm::mat4 p_cascade { glm::mat4(0.0f) };
    p_cascade[0][0] = 1.0f / df;
    p_cascade[1][1] = 1.0f / df;
    p_cascade[2][2] = -1.0f / z_range;
    p_cascade[3][2] = max_z / z_range;
    p_cascade[3][3] = 1.0f;

    glm::mat4 p_shadow { glm::mat4(0.0f) };
    p_shadow[0][0] = 0.5f / df;
    p_shadow[1][1] = -0.5f / df;
    p_shadow[2][2] = -1.0f / z_range;
    p_shadow[3][0] = 0.5f;
    p_shadow[3][1] = 0.5f;
    p_shadow[3][2] = max_z / z_range;
    p_shadow[3][3] = 1.0f;

    auto const render_mvp { p_cascade * m_cascade };
    auto const sampling_mvp { p_shadow * m_cascade };

    CSM csm {
      .d = static_cast<float>(d),
      .texel_size = (3.0f / 16.0f * (1.0f / shadow_resolution)),
      .z_max = max_z,
      .z_min = min_z,
      .s = snapped_world_center,
      .scale = glm::vec3(1.0f),
      .offset = glm::vec3(0.0f),
      .render = render_mvp,
      .sampling = sampling_mvp
    };

    return csm;
  }

  void LightManager::computeCSM(glm::mat4 const & camera_view, glm::mat4 const & projection)
  {
    float const near { -0.0f };
    float const far { -500.f };

  std::array<glm::vec2, 4> const cascade_splits {
      glm::vec2(near, -150.f),
      glm::vec2(-140.f, -280.f),
      glm::vec2(-270.f, -420.f),
      glm::vec2(-410.f, far)
  };

    std::vector<CSM> csms{};

    for (std::uint8_t i { 0 }; i < cascade_splits.size(); ++i) {
      csms.emplace_back(
        getLightSpaceMatrix(cascade_splits[i].x, cascade_splits[i].y, camera_view, projection));
    }

    auto const d0 { csms.at(0).d };
    auto const s0 { csms.at(0).s };
    auto const z0 {  csms.at(0).z_max  - csms.at(0).z_min };
    //auto const rot { glm::mat3(camera_view) };

    for (std::uint8_t i { 1 }; i < 4; ++i) {
      auto & csm { csms.at(i) };

      auto const z { csms.at(i).z_max  - csms.at(i).z_min };

      auto const scaleX { d0 / csm.d };
      auto const scaleZ { z0 / z };

      glm::vec3 world_delta { (s0 - csm.s) };

      auto const offsetX { world_delta.x / csm.d - (d0 / (2.f * csm.d)) + 0.5f };
      auto const offsetY { world_delta.y / csm.d - (d0 / (2.f * csm.d)) + 0.5f };
      auto const offsetZ { world_delta.z / z };

      csm.scale = glm::vec3(scaleX, scaleX, scaleZ);
      csm.offset = glm::vec3(offsetX, offsetY, offsetZ);
    }

    _sun.cascade_texel_size = csms.at(0).texel_size;

    _sun.cascade_min_splits.x = cascade_splits[0].x;
    _sun.cascade_min_splits.y = cascade_splits[1].x;
    _sun.cascade_min_splits.z = cascade_splits[2].x;
    _sun.cascade_min_splits.w = cascade_splits[3].x;

    _sun.cascade_max_splits.x = cascade_splits[0].y;
    _sun.cascade_max_splits.y = cascade_splits[1].y;
    _sun.cascade_max_splits.z = cascade_splits[2].y;
    _sun.cascade_max_splits.w = cascade_splits[3].y;

    _sun.cascade0  = csms.at(0).sampling;
    _sun.cascade_scale1 = csms.at(1).scale;
    _sun.cascade_scale2 = csms.at(2).scale;
    _sun.cascade_scale3 = csms.at(3).scale;

    _sun.cascade_offset1 = csms.at(1).offset;
    _sun.cascade_offset2 = csms.at(2).offset;
    _sun.cascade_offset3 = csms.at(3).offset;
    
    _sun.light_space_matrix = csms.at(0).render;
    _sun.light_space_matrix_left = csms.at(1).render;
    _sun.light_space_matrix_top = csms.at(2).render;
    _sun.light_space_matrix_right = csms.at(3).render;
    _sun.light_space_matrix_bottom = _sun.light_space_matrix;//unused
    _sun.light_space_matrix_back = _sun.light_space_matrix;//unused
  }

  void LightManager::animateAmbientLight(double const delta_time)
  {
    auto& light1 = _points.at(1);
    // light1.position.x += static_cast<float>(cos(glm::radians(delta_time * 360.0)));
    // light1.position.z += static_cast<float>(sin(glm::radians(delta_time * 360.0)));
    //light1.color *= delta_time;

     float fov = glm::radians(90.0f);
    glm::mat4 shadow_cubemap_projection = glm::perspective(fov, 1.0f, 1.0f, 50.f);
    shadow_cubemap_projection[1][1] *= -1;

    light1.projection = shadow_cubemap_projection;

    _ambient_hue += static_cast<float>(delta_time) * 0.1f;
    if (_ambient_hue > 1.0f) _ambient_hue -= 1.0f;

    light1.color = hsv2rgb(_ambient_hue, 1.0f, 1.0f);

    static double t { 0.0 };
    t += delta_time * 0.2;
    if (t > glm::two_pi<double>()) t -= glm::two_pi<double>();

    float const radius { 3.0f };
    float const y { light1.position.y };
    light1.position.x = radius * static_cast<float>(cos(t));
    light1.position.z = radius * static_cast<float>(sin(t));
    light1.position.y = y;

    light1.light_space_matrix_right = light1.projection * glm::lookAt(
      light1.position,
      light1.position + glm::vec3(1.0f, 0.0f, 0.0f),
      glm::vec3(0.0f, -1.0f, 0.0f));

    light1.light_space_matrix_left = light1.projection * glm::lookAt(
      light1.position,
      light1.position + glm::vec3(-1.0f, 0.0f, 0.0f),
      glm::vec3(0.0f, -1.0f, 0.0f));

    light1.light_space_matrix_top = light1.projection * glm::lookAt(
      light1.position,
      light1.position + glm::vec3(0.0f, 1.0f, 0.0f),
      glm::vec3(0.0f, 0.0f, 1.0f));

    light1.light_space_matrix_bottom = light1.projection * glm::lookAt(
      light1.position,
      light1.position + glm::vec3(0.0f, -1.0f, 0.0f),
      glm::vec3(0.0f, 0.0f, -1.0f));

    light1.light_space_matrix = light1.projection * glm::lookAt(
      light1.position,
      light1.position + glm::vec3(0.0f, 0.0f, 1.0f),
      glm::vec3(0.0f, -1.0f, 0.0f)); //used as light_space_matrix_front

    light1.light_space_matrix_back = light1.projection * glm::lookAt(
      light1.position,
      light1.position + glm::vec3(0.0f, 0.0f, -1.0f),
      glm::vec3(0.0f, -1.0f, 0.0f));

    light1.view = light1.light_space_matrix;
  }

  glm::vec3 LightManager::hsv2rgb(float h, float s, float v)
  {
    float c = v * s;
    float x = c * (1.f - fabsf(fmodf(h * 6.0f, 2.0f) - 1.f));
    float m = v - c;
    glm::vec3 rgb;

    if (h < 1.0f/6.0f)      rgb = glm::vec3(c, x, 0);
    else if (h < 2.0f/6.0f) rgb = glm::vec3(x, c, 0);
    else if (h < 3.0f/6.0f) rgb = glm::vec3(0, c, x);
    else if (h < 4.0f/6.0f) rgb = glm::vec3(0, x, c);
    else if (h < 5.0f/6.0f) rgb = glm::vec3(x, 0, c);
    else                    rgb = glm::vec3(c, 0, x);

    return rgb + glm::vec3(m);
  }

  void LightManager::animateSunLight(double const delta_time)
  {
    _sun.position.x += static_cast<float>(cos(glm::radians(delta_time * 360.0)));
    _sun.direction.x += static_cast<float>(cos(glm::radians(delta_time * 360.0)));
    _sun.position.z += static_cast<float>(sin(glm::radians(delta_time * 360.0)));
    _sun.direction.z += static_cast<float>(sin(glm::radians(delta_time * 360.0)));

    _sun.view = glm::lookAt(
      glm::vec3(_sun.position),
      glm::vec3(_sun.direction),
      glm::vec3(0.0f, 0.0f, -1.0f));

    float near_plane = 1.f, far_plane = 7.5f;
    _sun.projection = glm::ortho(-10.0f, 10.0f, 10.0f, -10.0f, near_plane, far_plane);

    _sun.light_space_matrix = _sun.projection * _sun.view;
  }
}
