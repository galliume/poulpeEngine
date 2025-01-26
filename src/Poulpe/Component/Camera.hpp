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

    inline glm::vec3 getPos() { return _camera_pos; }
    inline glm::mat4 getView() { return _view; }

    glm::mat4 frustumProj(float fovy, float s, float n, float f);
    glm::mat4 lookAt();
    void updateSpeed(double const delta_time);
    void updatePos(double x_offset, double y_offset);

  private:
    glm::vec3 mat4_backward();
    glm::vec3 mat4_down();
    glm::vec3 mat4_forward();
    glm::vec3 mat4_left();
    glm::vec3 mat4_right();
    glm::vec3 mat4_up();

  private:
    glm::vec3 _camera_front;
    glm::vec3 _camera_up;
    glm::vec3 _camera_pos;

    glm::mat4 _view;

    float _pitch;
    float const _pixel_distance{ 100.0f };
    float _speed { _pixel_distance };
    float _yaw{ -90.0f };
  };
}
