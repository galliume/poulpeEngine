module;
#include <memory>

export module Poulpe.Core.CommandQueueLocator;

import Poulpe.Core.CommandQueue;

namespace Poulpe
{
  export class CommandQueueLocator
  {
  public:
    static CommandQueue* get();
    static void init();

  private:
    static std::unique_ptr<CommandQueue> _commandQueue;
  };
}
