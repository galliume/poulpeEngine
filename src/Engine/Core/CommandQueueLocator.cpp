module;
#include <memory>

module Engine.Core.CommandQueueLocator;

import Engine.Core.CommandQueue;

namespace Poulpe
{
  std::unique_ptr<Poulpe::CommandQueue> Poulpe::CommandQueueLocator::_cmd_queue;
    
  CommandQueue* CommandQueueLocator::get()
  { 
    return _cmd_queue.get();
  }

  void CommandQueueLocator::init()
  {
    _cmd_queue = std::make_unique<CommandQueue>();
  }
}
