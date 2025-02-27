#pragma once

#include <queue>
#include <memory>

#include "Command.hpp"

#include <mutex>

namespace Poulpe
{
    class CommandQueue
    {
    public:
        CommandQueue();
        ~CommandQueue() = default;

        void add(Command& cmd);
        void execPostRequest();
        void execPreRequest();

    private:
        mutable std::mutex _Mutex;

        std::queue<std::shared_ptr<Command>> _PostCmdQueue;
        std::queue<std::shared_ptr<Command>> _PreCmdQueue;
    };
}