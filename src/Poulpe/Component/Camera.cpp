#include "Camera.hpp"

namespace Poulpe
{
  void Camera::init()
  {
    _Pos = glm::vec3(-0.2f, 0.2f, 0.0f);
    _Target = glm::vec3(0.0f, 0.0f, 0.0f);
    _Direction = glm::normalize(_Pos - _Target);

    _CameraUp = glm::vec3(0.0f, 1.0f,  0.0f);
    _CameraFront = glm::vec3(0.0f, 0.0f, -1.0f);

    _View = glm::lookAt(_Pos, _Pos + _CameraFront, _CameraUp);
  }

  void Camera::backward()
  {
    //auto direction = mat4_backward();
    //_Pos -= direction * (_Speed * _DeltaTime.count());
    _Pos -= _CameraFront * _Speed;
  }

  void Camera::down()
  {
    _Pos += _CameraUp * _Speed;
  }

  void Camera::forward()
  {
    _Pos += _CameraFront * _Speed;
  }

  void Camera::left()
  {
    _Pos -= glm::normalize(glm::cross(_CameraFront, _CameraUp)) * _Speed;
  }

  void Camera::right()
  {
    _Pos += glm::normalize(glm::cross(_CameraFront, _CameraUp)) * _Speed;
  }

  void Camera::up()
  {
    _Pos -= _CameraUp * _Speed;
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
    _View = glm::lookAt(_Pos, _Pos + _CameraFront, _CameraUp);
    return _View;
  }

  void Camera::updateSpeed(std::chrono::duration<float> deltaTime)
  {
    _Speed = _velocity * deltaTime.count();
  }

  void Camera::updatePos(double xoffset, double yoffset)
  {
    _Yaw += static_cast<float>(xoffset);
    _Pitch += static_cast<float>(yoffset);

    if (_Pitch > 89.0f) {
        _Pitch = 89.0f;
    }
    if (_Pitch < -89.0f) {
        _Pitch = -89.0f;
    }

    glm::vec3 direction{};
    direction.x = static_cast<float>(cos(glm::radians(_Yaw)) * cos(glm::radians(_Pitch)));
    direction.y = static_cast<float>(sin(glm::radians(_Pitch)));
    direction.z = static_cast<float>(sin(glm::radians(_Yaw)) * cos(glm::radians(_Pitch)));
    _CameraFront = glm::normalize(direction);
  }

  glm::vec3 Camera::mat4_backward()
  {
    glm::vec3 vec;
    vec.x = _View[2][0];
    vec.y = _View[2][1];
    vec.y = _View[2][2];
    return vec;
  }
  glm::vec3 Camera::mat4_down()
  {
    glm::vec3 vec;
    vec.x = -_View[1][0];
    vec.y = -_View[1][1];
    vec.y = -_View[1][2];
    return vec;
  }
  glm::vec3 Camera::mat4_forward()
  {
    glm::vec3 vec;
    vec.x = -_View[2][0];
    vec.y = -_View[2][1];
    vec.y = -_View[2][2];
    return vec;
  }
  glm::vec3 Camera::mat4_left()
  {
    glm::vec3 vec;
    vec.x = -_View[2][0];
    vec.y = -_View[2][1];
    vec.y = -_View[2][2];
    return vec;
  }
  glm::vec3 Camera::mat4_right()
  {
    glm::vec3 vec;
    vec.x = _View[2][0];
    vec.y = _View[2][1];
    vec.y = _View[2][2];
    return vec;
  }
  glm::vec3 Camera::mat4_up()
  {
    glm::vec3 vec;
    vec.x = _View[0][0];
    vec.y = _View[0][1];
    vec.y = _View[0][2];
    return vec;
  }
}
