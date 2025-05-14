module;
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <numbers>

module Poulpe.Component.Camera;

namespace Poulpe
{
  void Camera::init(glm::vec3 const& start_pos)
  {
    _next_front = glm::normalize(glm::vec3(1.0, 0.0, 1.0) * glm::vec3(2.0 * std::numbers::pi / 1000.0));
    _camera_pos = _next_pos = start_pos;
    _camera_up = glm::vec3(0.0f, 1.0f,  0.0f);
    _camera_front = glm::vec3(0.0f, 0.0f, -1.0f);

    _view = glm::lookAt(_camera_pos, _camera_pos + _camera_front, _camera_up);
    _init = true;
  }

  void Camera::backward()
  {
    _next_pos -= (_camera_front * _acceleration * getDeltaTime());
  }

  void Camera::down()
  {
    _next_pos += (_camera_up * _acceleration * getDeltaTime());
  }

  void Camera::forward()
  {
    _next_pos += (_camera_front * _acceleration * getDeltaTime());
  }

  void Camera::left()
  {
    _next_pos += glm::normalize(glm::cross(_camera_front, _camera_up)) * _acceleration * getDeltaTime();
  }

  void Camera::right()
  {
    _next_pos -= glm::normalize(glm::cross(_camera_front, _camera_up)) * _acceleration * getDeltaTime();
  }

  void Camera::up()
  {
    _next_pos -= (_camera_up * _acceleration * getDeltaTime());
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
    return glm::vec3(_delta_time);
  }

  glm::mat4 Camera::lookAt()
  {
    _view = glm::lookAt(_camera_pos, _camera_pos + _camera_front, _camera_up);
    return _view;
  }

  void Camera::updateAngle(double const xoffset, double const yoffset)
  {
    _yaw -= static_cast<float>(xoffset);
    _pitch += static_cast<float>(yoffset);

    if (_pitch > 89.0f) {
      _pitch = 89.0f;
    }
    if (_pitch < -89.0f) {
      _pitch = -89.0f;
    }

    glm::vec3 direction{};
    direction.x = static_cast<float>(_acceleration * cos(glm::radians(_yaw)) * cos(glm::radians(_pitch)));
    direction.y = static_cast<float>(_acceleration * sin(glm::radians(_pitch)));
    direction.z = static_cast<float>(_acceleration * sin(glm::radians(_yaw)) * cos(glm::radians(_pitch)));

    _next_front = glm::normalize(direction * glm::vec3(2.0 * std::numbers::pi / 1000.0));
  }

  void Camera::move()
  {
    _camera_front = _next_front;
    _camera_pos = _next_pos;
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
