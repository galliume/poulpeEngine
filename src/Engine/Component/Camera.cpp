module;
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/fwd.hpp>

module Engine.Component.Camera;

import std;

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

  glm::mat4 Camera::lookAt()
  {
    _view = glm::lookAt(_pos_rendered, _pos_rendered + _front, _up);
    return _view;
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
      static_cast<float>(cos(glm::radians(_yaw)) * cos(glm::radians(_pitch))),
      static_cast<float>(sin(glm::radians(_pitch))),
      static_cast<float>(sin(glm::radians(_yaw)) * cos(glm::radians(_pitch))) };

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
}
