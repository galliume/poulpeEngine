module;

export module Engine.Core.Camera;

import std;

import Engine.Core.GLM;

namespace Poulpe
{
  export enum class CameraType : std::uint8_t
  {
    FREE,
    THIRD_PERSON
  };

  export class Camera
  {
  public:
    void init(glm::vec3 const& start_pos);

    void backward();
    void down();
    void forward();
    void left();
    void right();
    void up();

    inline glm::vec3 getPos() const { return _pos; }
    glm::mat4 getView() const;
    glm::mat4 getThirdPersonView(glm::vec3 const & player_pos);
    inline glm::vec3 getDirection() const { return _front; }
    void setPos(glm::vec3 const & pos) { _pos = pos; _next_pos = pos; }
    glm::mat4 frustumProj(float fovy, float s, float n, float f);
    void updateDeltaTime(double const delta_time) { _delta_time = delta_time; }
    void updateAngle(double const x_offset, double const y_offset);
    void move();

    bool isInit() const { return _init; }

    glm::vec3 getDeltaTime();

    void savePreviousState() { _pos_previous = _pos; }
    void interpolate(double alpha) {
          _pos_rendered = glm::mix(_pos_previous, _pos, alpha);
      }

    std::vector<glm::vec4> getFrustumPlanes(glm::mat4 const vp);

    private:
    glm::vec3 mat4_backward();
    glm::vec3 mat4_down();
    glm::vec3 mat4_forward();
    glm::vec3 mat4_left();
    glm::vec3 mat4_right();
    glm::vec3 mat4_up();

  private:
    glm::vec3 _front { 1.0f, 1.0f, 1.0f };
    glm::vec3 _up { 0.0f, 1.0f, 0.0f };
    glm::vec3 _world_up { 1.0f, 1.0f, 1.0f };
    glm::vec3 _right { 1.0f, 1.0f, 1.0f };
    glm::vec3 _pos { 1.0f, 1.0f, 1.0f };

    glm::vec3 _pos_previous;
    glm::vec3 _pos_rendered;

    glm::vec3 _next_pos;
    glm::vec3 _next_front;

    glm::mat4 _view;

    double _pitch;
    double _yaw{ -90.0 };
    float const _acceleration{ 200.f };
    double _delta_time{ 0.0 };

    bool _init{ false };
  };
}
