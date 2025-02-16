#pragma once

#include <glm/glm.hpp>

namespace Poulpe
{
  class Camera
  {
  public:
    void init(glm::vec3 const& start_pos);

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
    void updateDeltaTime(double const delta_time) { _delta_time = delta_time; };
    void updateAngle(double const x_offset, double const y_offset);
    void move();

    bool const isInit() { return _init; };

    glm::vec3 getDeltaTime();

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

    glm::vec3 _next_pos;
    glm::vec3 _next_front;

    glm::mat4 _view;

    float _pitch;
    float _yaw{ -90.0f };
    float const _acceleration{ 100.f };
    double _delta_time{ 0.0 };

    bool _init{ false };
  };
}
