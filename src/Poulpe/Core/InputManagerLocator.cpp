module;
#include <memory>

module Poulpe.Core.InputManagerLocator;

namespace Poulpe
{
  InputManager* InputManagerLocator::get()
  {
    return _inputManager.get();
  }

  void InputManagerLocator::init(Window const * const window)
  {
    _inputManager = std::make_unique<InputManager>(window);
  }
}
