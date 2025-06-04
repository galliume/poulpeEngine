module;
#include <memory>

module Poulpe.Core.CommandQueueLocator;

import Poulpe.Core.CommandQueue;

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
