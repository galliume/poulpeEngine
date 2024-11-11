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

    inline glm::vec3 getDirection() { return _Direction; }
    inline glm::vec4 getPos() { return glm::vec4(_Pos, 1.0f); }
    inline glm::mat4 getView() { return _View; }

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
    glm::vec3 _Direction;
    glm::vec3 _CameraFront;
    glm::vec3 _CameraUp;
    glm::vec3 _Pos;
    glm::vec3 _Target;

    glm::mat4 _View;

    float _Pitch;
    float _velocity{ 10.0f };
    float _Speed{ _velocity };
    float _Yaw{ -90.0f };
  };
}
