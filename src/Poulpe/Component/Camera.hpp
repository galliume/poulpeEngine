#pragma once

#include <glm/glm.hpp>

namespace Poulpe
{
  class Camera
  {
  public:
    void init();

    void backward();
    void down();
    void forward();
    void left();
    void right();
    void up();

    inline glm::vec3 getDirection() { return _direction; }
    inline glm::vec4 getPos() { return glm::vec4(_position, 1.0f); }
    inline glm::mat4 getView() { return _view; }

    glm::mat4 frustumProj(float fovy, float s, float n, float f);
    glm::mat4 lookAt();
    void updateSpeed(std::chrono::duration<float> deltaTime);
    void updatePos(double xoffset, double yoffset);

  private:
    glm::vec3 mat4_backward();
    glm::vec3 mat4_down();
    glm::vec3 mat4_forward();
    glm::vec3 mat4_left();
    glm::vec3 mat4_right();
    glm::vec3 mat4_up();

  private:
    glm::vec3 _direction;
    glm::vec3 _camera_front;
    glm::vec3 _camera_up;
    glm::vec3 _position;
    glm::vec3 _target;

    glm::mat4 _view;

    float _pitch;
    float _velocity{ 10.0f };
    float _speed{ _velocity };
    float _yaw{ -90.0f };
  };
}
