#include "Camera.hpp"

namespace Poulpe
{
  void Camera::init()
  {
    _position = glm::vec3(-0.2f, 0.2f, 0.0f);
    _target = glm::vec3(0.0f, 0.0f, 0.0f);
    _direction = glm::normalize(_position - _target);

    _camera_up = glm::vec3(0.0f, 1.0f,  0.0f);
    _camera_front = glm::vec3(0.0f, 0.0f, -1.0f);

    _view = glm::lookAt(_position, _position + _camera_front, _camera_up);
  }

  void Camera::backward()
  {
    //auto direction = mat4_backward();
    //_position -= direction * (_speed * _DeltaTime.count());
    _position -= _camera_front * _speed;
  }

  void Camera::down()
  {
    _position += _camera_up * _speed;
  }

  void Camera::forward()
  {
    _position += _camera_front * _speed;
  }

  void Camera::left()
  {
    _position -= glm::normalize(glm::cross(_camera_front, _camera_up)) * _speed;
  }

  void Camera::right()
  {
    _position += glm::normalize(glm::cross(_camera_front, _camera_up)) * _speed;
  }

  void Camera::up()
  {
    _position -= _camera_up * _speed;
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

  glm::mat4 Camera::lookAt()
  {
    _view = glm::lookAt(_position, _position + _camera_front, _camera_up);
    return _view;
  }

  void Camera::updateSpeed(std::chrono::duration<float> deltaTime)
  {
    _speed = _velocity * deltaTime.count();
  }

  void Camera::updatePos(double xoffset, double yoffset)
  {
    _yaw += static_cast<float>(xoffset);
    _pitch += static_cast<float>(yoffset);

    if (_pitch > 89.0f) {
      _pitch = 89.0f;
    }
    if (_pitch < -89.0f) {
      _pitch = -89.0f;
    }

    glm::vec3 direction{};
    direction.x = static_cast<float>(cos(glm::radians(_yaw)) * cos(glm::radians(_pitch)));
    direction.y = static_cast<float>(sin(glm::radians(_pitch)));
    direction.z = static_cast<float>(sin(glm::radians(_yaw)) * cos(glm::radians(_pitch)));
    _camera_front = glm::normalize(direction);
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
