module;
#include <memory>

module Poulpe.Managers.InputManagerLocator;

namespace Poulpe
{
  InputManager* InputManagerLocator::get()
  {
    return _input_manager.get();
  }

  void InputManagerLocator::init(Window const * const window)
  {
    _input_manager = std::make_unique<InputManager>(window);
  }
}
