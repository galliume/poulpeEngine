module;

#include <memory>
#include <mutex>
#include <queue>

export module Poulpe.Core.CommandQueue;

import Poulpe.Core.Command;

export class CommandQueue
{
public:
    CommandQueue() = default;
    ~CommandQueue() = default;

    void add(Command& cmd);
    void execPostRequest();
    void execPreRequest();

private:
    std::mutex _Mutex;

    std::queue<std::shared_ptr<Command>> _PostCmdQueue;
    std::queue<std::shared_ptr<Command>> _PreCmdQueue;
};
