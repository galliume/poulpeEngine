module Engine.Core.CommandQueueLocator;

import std;

import Engine.Core.CommandQueue;

namespace Poulpe
{
  Poulpe::CommandQueue* Poulpe::CommandQueueLocator::_cmd_queue = nullptr;

  CommandQueue* CommandQueueLocator::get()
  {
    return _cmd_queue;
  }
  
  void CommandQueueLocator::init()
  {
    if (!_cmd_queue) {
      _cmd_queue = new Poulpe::CommandQueue();
    }
  }
}
