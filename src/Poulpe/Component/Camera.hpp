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

    inline glm::vec3 getDirection() { return m_Direction; }
    inline glm::vec4 getPos() { return glm::vec4(m_Pos, 1.0f); }
    inline glm::mat4 getView() { return m_View; }

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
    glm::vec3 m_Direction;
    glm::vec3 m_CameraFront;
    glm::vec3 m_CameraUp;
    glm::vec3 m_Pos;
    glm::vec3 m_Target;

    glm::mat4 m_View;

    float m_Pitch;
    float m_velocity{ 10.0f };
    float m_Speed{ m_velocity };
    float m_Yaw{ -90.0f };
  };
}
