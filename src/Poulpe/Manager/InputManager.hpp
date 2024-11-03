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
    void inline setCamera(Camera * const camera) { m_Camera = camera; };
    void updateMousePos(double xPos, double yPo);

    static bool m_CanMoveCamera;
    static bool m_FirtMouseMove;

  private:
    double m_LastX{ 0 };
    double m_LastY{ 0 };

    //@todo clean this
    Camera * m_Camera;
    Window const * const m_Window;

    nlohmann::json m_InputConfig;
    std::unordered_map<std::string, int> m_KeyboardKeys{};
  };
}
