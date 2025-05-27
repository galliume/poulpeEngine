module;
#include <memory>

module Poulpe.Managers.InputManagerLocator;

import Poulpe.GUI.Window;
import Poulpe.Managers;

namespace Poulpe
{
   [[clang::no_destroy]] std::unique_ptr<InputManager> InputManagerLocator::_input_manager;

  InputManager* InputManagerLocator::get()
  {
    return _input_manager.get();
  }

  void InputManagerLocator::init(Window const * const window)
  {
    _input_manager = std::make_unique<InputManager>(window);
  }
}
