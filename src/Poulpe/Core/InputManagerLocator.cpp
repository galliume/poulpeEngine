module;
#include <memory>

module Poulpe.Core.InputManagerLocator;

namespace Poulpe
{
  std::unique_ptr<Poulpe::InputManager> Poulpe::InputManagerLocator::_input_manager;

  InputManager* InputManagerLocator::get()
  {
    return _input_manager.get();
  }

  void InputManagerLocator::init(Window const * const window)
  {
    _input_manager = std::make_unique<InputManager>(window);
  }
}
