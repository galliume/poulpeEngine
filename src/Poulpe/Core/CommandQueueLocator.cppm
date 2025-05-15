module;
#include <memory>

export module Poulpe.Core.CommandQueueLocator;

import Poulpe.Core.CommandQueue;

export class CommandQueueLocator
{
public:
  static CommandQueue* get() { return _commandQueue.get(); }
  
  static void init()
  {
    _commandQueue = std::make_unique<CommandQueue>();
  }

private:
  inline static std::unique_ptr<CommandQueue> _commandQueue;
};
