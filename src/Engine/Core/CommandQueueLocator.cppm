export module Engine.Core.CommandQueueLocator;

import std;

import Engine.Core.CommandQueue;

namespace Poulpe
{
  export class CommandQueueLocator
  {
  public:
    static CommandQueue* get();
    static void init();

  private:
    static Poulpe::CommandQueue* _cmd_queue;
  };
}
