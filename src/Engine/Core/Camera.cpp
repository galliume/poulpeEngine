module Engine.Core.Camera;

import std;

import Engine.Core.Logger;
import Engine.Core.GLM;

namespace Poulpe
{
  void Camera::init(glm::vec3 const& start_pos)
  {
    _world_up = glm::vec3(0.0f, 1.0f, 0.0f);
    _front = glm::vec3(0.0f, 0.0f, 1.0f);
    _pitch = 0.0;
    _yaw = 0.0;
    _init = true;
    _next_pos = start_pos;
    _next_front = _front;
  }

  void Camera::backward()
  {
    _next_pos -= _front * (_acceleration * getDeltaTime());
  }

  void Camera::down()
  {
    _next_pos += _up * (_acceleration * getDeltaTime());
  }

  void Camera::forward()
  {
    _next_pos += _front * (_acceleration * getDeltaTime());
  }

  void Camera::left()
  {
    _next_pos -= _right * (_acceleration * getDeltaTime());
  }

  void Camera::right()
  {
    _next_pos += _right * (_acceleration * getDeltaTime());
  }

  void Camera::up()
  {
    _next_pos -= _up * (_acceleration * getDeltaTime());
  }

  glm::mat4 Camera::getView() const
  {
    return glm::lookAt(_pos, _pos + _front, _up);
  }
  
  glm::mat4 Camera::getThirdPersonView(glm::vec3 const & player_pos)
  {
    return glm::lookAt(_pos, player_pos + _front, _up);
  }

  /**
      fovy : viertical field of view
      s : aspect ratio viewport
      n : near distance
      f : far distance
  **/
  glm::mat4 Camera::frustumProj(float fovy, float s, float n, float f)
  {
    //infine reverse frustum projection
    /*float g = 1.0f / tan(fovy * 0.5f);
    float e = 0.1f;

    frustumProj = glm::mat4(
        g / s, 0.0f, 0.0f, 0.0f,
        0.0f, g, 0.0f, 0.0f,
        0.0f, 0.0f, e, n * (1.0f - e),
        0.0f, 0.0f, 1.0f, 0.0f
    );*/

    //infine frustum projection
    //glm::mat4 frustumProj;
    //float g = 1.0f / tan(fovy * 0.5f);
    //float e = 1.0f - 0.00001f;

    //frustumProj = glm::mat4(
    //	g / s, 0.0f, 0.0f, 0.0f,
    //	0.0f, g, 0.0f, 0.0f,
    //	0.0f, 0.0f, e, -n * e,
    //	0.0f, 0.0f, 1.0f, 0.0f
    //);

    //frustum projection
    float g = 1.0f / std::tan(fovy * 0.5f);
    float k = f / (f - n);

    glm::mat4 frustumProj = glm::mat4(
      g / s, 0.0f, 0.0f, 0.0f,
      0.0f, g, 0.0f, 0.0f,
      0.0f, 0.0f, k, -n * k,
      0.0f, 0.0f, 1.0f, 0.0f
    );

    //reverse frustum projection
    //float g = 1.0f / std::tan(fovy * 0.5f);
    //float k = n / (n - f);

    //glm::mat4 frustumProj = glm::mat4(
    //  g / s, 0.0f, 0.0f, 0.0f,
    //  0.0f, g, 0.0f, 0.0f,
    //  0.0f, 0.0f, k, -f * k,
    //  0.0f, 0.0f, 1.0f, 0.0f
    //);
    //float k = f / (f - n);

    //float g = 1.0f / tan(fovy * 0.5f);
    //float e = 1.0f - 0.0f;

    //glm::mat4 frustumProj = glm::mat4(0.0f);
    //frustumProj[0][0] = g / s;
    //frustumProj[1][1] = g;
    //frustumProj[2][2] = e;
    //frustumProj[2][3] = -n * e;
    //frustumProj[3][3] = 1.0f;

    return frustumProj;
  }

  glm::vec3 Camera::getDeltaTime()
  {
    return glm::vec3(static_cast<float>(_delta_time));
  }

  void Camera::updateAngle(double const xoffset, double const yoffset)
  {
    _yaw += xoffset;
    _pitch += yoffset;

    if (_pitch > 89.0) {
      _pitch = 89.0;
    }
    if (_pitch < -89.0) {
      _pitch = -89.0;
    }

    glm::vec3 const front {
      static_cast<float>(glm::cos(glm::radians(_yaw)) * glm::cos(glm::radians(_pitch))),
      static_cast<float>(glm::sin(glm::radians(_pitch))),
      static_cast<float>(glm::sin(glm::radians(_yaw)) * glm::cos(glm::radians(_pitch))) };

    _next_front = glm::normalize(front);
  }

  void Camera::move()
  {
    _front =_next_front;
    _pos = _next_pos;
    _right = glm::normalize(glm::cross(_front, _world_up));
    _up = glm::normalize(glm::cross(_right, _front));
  }

  glm::vec3 Camera::mat4_backward()
  {
    glm::vec3 vec;
    vec.x = _view[2][0];
    vec.y = _view[2][1];
    vec.y = _view[2][2];
    return vec;
  }
  glm::vec3 Camera::mat4_down()
  {
    glm::vec3 vec;
    vec.x = -_view[1][0];
    vec.y = -_view[1][1];
    vec.y = -_view[1][2];
    return vec;
  }
  glm::vec3 Camera::mat4_forward()
  {
    glm::vec3 vec;
    vec.x = -_view[2][0];
    vec.y = -_view[2][1];
    vec.y = -_view[2][2];
    return vec;
  }
  glm::vec3 Camera::mat4_left()
  {
    glm::vec3 vec;
    vec.x = -_view[2][0];
    vec.y = -_view[2][1];
    vec.y = -_view[2][2];
    return vec;
  }
  glm::vec3 Camera::mat4_right()
  {
    glm::vec3 vec;
    vec.x = _view[2][0];
    vec.y = _view[2][1];
    vec.y = _view[2][2];
    return vec;
  }
  glm::vec3 Camera::mat4_up()
  {
    glm::vec3 vec;
    vec.x = _view[0][0];
    vec.y = _view[0][1];
    vec.y = _view[0][2];
    return vec;
  }

  std::vector<glm::vec4> Camera::getFrustumPlanes(glm::mat4 const vp)
  {
    std::vector<glm::vec4> frustum_planes(6);

    frustum_planes[0].x = vp[0].w + vp[0].x;
    frustum_planes[0].y = vp[1].w + vp[1].x;
    frustum_planes[0].z = vp[2].w + vp[2].x;
    frustum_planes[0].w = vp[3].w + vp[3].x;

    frustum_planes[1].x = vp[0].w - vp[0].x;
    frustum_planes[1].y = vp[1].w - vp[1].x;
    frustum_planes[1].z = vp[2].w - vp[2].x;
    frustum_planes[1].w = vp[3].w - vp[3].x;

    frustum_planes[2].x = vp[0].w - vp[0].y;
    frustum_planes[2].y = vp[1].w - vp[1].y;
    frustum_planes[2].z = vp[2].w - vp[2].y;
    frustum_planes[2].w = vp[3].w - vp[3].y;

    frustum_planes[3].x = vp[0].w + vp[0].y;
    frustum_planes[3].y = vp[1].w + vp[1].y;
    frustum_planes[3].z = vp[2].w + vp[2].y;
    frustum_planes[3].w = vp[3].w + vp[3].y;

    frustum_planes[4].x = vp[0].w + vp[0].z;
    frustum_planes[4].y = vp[1].w + vp[1].z;
    frustum_planes[4].z = vp[2].w + vp[2].z;
    frustum_planes[4].w = vp[3].w + vp[3].z;

    frustum_planes[5].x = vp[0].w - vp[0].z;
    frustum_planes[5].y = vp[1].w - vp[1].z;
    frustum_planes[5].z = vp[2].w - vp[2].z;
    frustum_planes[5].w = vp[3].w - vp[3].z;

    // for (std::size_t i { 0 }; i < frustum_planes.size(); i++)
    // {
    //   float length = sqrtf(frustum_planes[i].x * frustum_planes[i].x + frustum_planes[i].y * frustum_planes[i].y + frustum_planes[i].z * frustum_planes[i].z);
    //   frustum_planes[i] /= length;
    // }

    for (auto& plane : frustum_planes) {
      plane /= glm::length(glm::vec3(plane));
    }
    
    return frustum_planes;
    // auto const& appConfig { config_manager->appConfig()["resolution"] };
    // auto const g { perspective[1][1] };
    // auto const s { appConfig["width"].get<std::uint32_t>() / appConfig["height"].get<std::uint32_t>() };
    // auto const near_plane { 0.01f };
    // auto const far_plane { 1000.f };

    // auto const M_camera_inv { perspective * camera_view_matrix };

    // auto const near_h { near_plane / g };
    // auto const near_w { near_h * s };
    // auto const near_v0 { glm::vec3(-near_w, near_h, -near_plane) };
    // auto const near_v1 { glm::vec3(near_w, near_h, -near_plane) };
    // auto const near_v2 { glm::vec3(near_w, -near_h, -near_plane) };
    // auto const near_v3 { glm::vec3(-near_w, -near_h, -near_plane) };

    // auto const far_h { far_plane / g };
    // auto const far_w { far_h * s };
    // auto const far_v0 { glm::vec3(-far_w, far_h, -far_plane) };
    // auto const far_v1 { glm::vec3(far_w, far_h, -far_plane) };
    // auto const far_v2 { glm::vec3(far_w, -far_h, -far_plane) };
    // auto const far_v3 { glm::vec3(-far_w, -far_h, -far_plane) };

    // std::vector<glm::vec4> frustum_planes {
    //     M_camera_inv * glm::vec4(near_v0, 1.0f),
    //     M_camera_inv * glm::vec4(near_v1, 1.0f),
    //     M_camera_inv * glm::vec4(near_v2, 1.0f),
    //     M_camera_inv * glm::vec4(near_v3, 1.0f),
    //     M_camera_inv * glm::vec4(far_v0, 1.0f),
    //     M_camera_inv * glm::vec4(far_v1, 1.0f),
    //     M_camera_inv * glm::vec4(far_v2, 1.0f),
    //     M_camera_inv * glm::vec4(far_v3, 1.0f)
    // };

    // for (auto& v : frustum_planes) {
    //     v /= v.w;
    // }
    // const auto inv { glm::inverse(perspective * camera_view_matrix) };
    
    // std::vector<glm::vec4> frustum_planes;
    // for (unsigned int x = 0; x < 2; ++x) {
    //   for (unsigned int y = 0; y < 2; ++y) {
    //     for (unsigned int z = 0; z < 2; ++z) {
    //       const glm::vec4 pt = 
    //         inv * glm::vec4(
    //             2.0f * static_cast<float>(x) - 1.0f,
    //             2.0f * static_cast<float>(y) - 1.0f,
    //             2.0f * static_cast<float>(z) - 1.0f,
    //             1.0f);
    //       frustum_planes.push_back(pt / pt.w);
    //     }
    //   }
    // }
  }
}
