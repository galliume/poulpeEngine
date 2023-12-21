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

        void add(Command& cmd);
        void execPostRequest();
        void execPreRequest();

    private:
        mutable std::mutex m_Mutex;

        std::queue<std::shared_ptr<Command>> m_PostCmdQueue;
        std::queue<std::shared_ptr<Command>> m_PreCmdQueue;
    };
}