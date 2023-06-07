#include "CommandQueue.hpp"

namespace Rbk
{
    CommandQueue::CommandQueue()
    {

    }

    void CommandQueue::Add(Command& cmd)
    {
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            std::shared_ptr<Command> cmdToQueue(std::make_shared<Command>(std::move(cmd)));
            m_CmdQueue.push(cmdToQueue);
        }
    }
}