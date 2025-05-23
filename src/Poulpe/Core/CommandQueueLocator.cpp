module;
#include <memory>

module Poulpe.Core.CommandQueueLocator;

namespace Poulpe
{
  CommandQueue* CommandQueueLocator::get()
  { 
    return _commandQueue.get();
  }

  void CommandQueueLocator::init()
  {
    _commandQueue = std::make_unique<CommandQueue>();
  }
}
