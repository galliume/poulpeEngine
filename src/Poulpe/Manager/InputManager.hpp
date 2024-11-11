#pragma once

#include "Poulpe/Component/Camera.hpp"

#include "Poulpe/GUI/Window.hpp"

#include <nlohmann/json.hpp>

namespace Poulpe
{
  class InputManager
  {
  public:
    explicit InputManager(Window const * const window);

    virtual void init(nlohmann::json const& inputConfig) ;

    void key(int key, int scancode, int action, int mods);
    void mouseButton(int button, int action, int mods);
    void saveLastMousePos(double xPos, double yPos);
    void inline setCamera(Camera * const camera) { _camera = camera; };
    void updateMousePos(double xPos, double yPo);

    static bool _CanMoveCamera;
    static bool _FirtMouseMove;

  private:
    double _LastX{ 0 };
    double _LastY{ 0 };

    //@todo clean this
    Camera * _camera;
    Window const * const _window;

    nlohmann::json _InputConfig;
    std::unordered_map<std::string, int> _KeyboardKeys{};
  };
}
