module;

#include <memory>
#include <mutex>
#include <vector>

export module Engine.Core.CommandQueue;

import Engine.Core.Command;

namespace Poulpe
{
  export class CommandQueue
  {
  public:
    // CommandQueue() = default;
    // ~CommandQueue() = default;

    // void add(Command& cmd);
    // void execPostRequest();
    // void execPreRequest();

  private:
    std::mutex _mutex;

    //@todo why std::queue does not work since module?
    // std::vector<std::shared_ptr<Command>> _postCmdQueue;
    // std::vector<std::shared_ptr<Command>> _preCmdQueue;
  };
}
