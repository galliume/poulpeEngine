#pragma once

#include <queue>
#include <memory>

#include "Command.hpp"

namespace Poulpe
{
    class CommandQueue
    {
    public:
        CommandQueue();
        ~CommandQueue() = default;

        void Add(Command& cmd);
        void ExecRequest();

    private:
        mutable std::mutex m_Mutex;
        std::queue<std::shared_ptr<Command>> m_CmdQueue;
    };
}