module;
#include <memory>

export module Poulpe.Managers.InputManagerLocator;

import Poulpe.Managers;

import Poulpe.GUI.Window;

namespace Poulpe
{
  export class InputManagerLocator
  {
  public:
    static InputManager* get();
    static void init(Window const * const window);

  private:
    static std::unique_ptr<InputManager> _input_manager;
  };
}
