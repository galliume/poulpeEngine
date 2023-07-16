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

    void CommandQueue::ExecRequest()
    {
        Rbk::Locator::getThreadPool()->Submit("commandQueue", [=, this]() {

            {
                std::lock_guard guard(m_Mutex);

                while (!m_CmdQueue.empty())
                {
                    std::shared_ptr<Command> cmd = m_CmdQueue.front();
                    cmd->ExecRequest();

                    m_CmdQueue.pop();
                }
            }
        });
    }
}