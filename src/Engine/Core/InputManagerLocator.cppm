export module Engine.Core.InputManagerLocator;

import std;

import Engine.GUI.Window;
import Engine.Manager.InputManager;

namespace Poulpe
{
  export class InputManagerLocator
  {
  public:
    InputManagerLocator() = default;
    ~InputManagerLocator() = default;

    static InputManager* get();
    static void init(Window const * const window);

  private:
    static std::unique_ptr<InputManager> _input_manager;
  };
}
