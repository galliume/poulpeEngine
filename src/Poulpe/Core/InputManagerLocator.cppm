module;
#include <memory>

export module Poulpe.Core.InputManagerLocator;

import Poulpe.GUI.Window;
import Poulpe.Manager.InputManager;

export class InputManagerLocator
{
public:
  static InputManager* get() { return _inputManager.get(); }

  static void init(Window const * const window)
  {
    _inputManager = std::make_unique<InputManager>(window);
  }

private:
  inline static std::unique_ptr<InputManager> _inputManager;
};
