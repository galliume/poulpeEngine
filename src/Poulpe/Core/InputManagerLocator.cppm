module;
#include <memory>

export module Poulpe.Core.InputManagerLocator;

import Poulpe.GUI.Window;
import Poulpe.Manager.InputManager;

namespace Poulpe
{
  export class InputManagerLocator
  {
  public:
    static InputManager* get();
    static void init(Window const * const window);

  private:
    static std::unique_ptr<InputManager> _inputManager;
  };
}
