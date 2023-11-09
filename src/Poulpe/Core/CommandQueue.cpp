#include "CommandQueue.hpp"

namespace Poulpe
{
    CommandQueue::CommandQueue()
    {

    }

    void CommandQueue::add(Command& cmd)
    {
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            std::shared_ptr<Command> cmdToQueue(std::make_shared<Command>(std::move(cmd)));

            if (cmd.getWhenToExecute() == WhenToExecute::PRE_RENDERING) m_PreCmdQueue.push(cmdToQueue);
            else if (cmd.getWhenToExecute() == WhenToExecute::POST_RENDERING) m_PostCmdQueue.push(cmdToQueue);
        }
    }

    void CommandQueue::execPreRequest()
    {
        Poulpe::Locator::getThreadPool()->submit("preCommandQueue", [=, this]() {
            {
                std::lock_guard guard(m_Mutex);

                while (!m_PreCmdQueue.empty())
                {
                    std::shared_ptr<Command> cmd = m_PreCmdQueue.front();
                    cmd->execRequest();
                    m_PreCmdQueue.pop();
                }
            }
        });
    }

    void CommandQueue::execPostRequest()
    {
      Poulpe::Locator::getThreadPool()->submit("postCommandQueue", [=, this]() {
        {
          std::lock_guard guard(m_Mutex);

          while (!m_PostCmdQueue.empty())
          {
            std::shared_ptr<Command> cmd = m_PostCmdQueue.front();
            cmd->execRequest();
            m_PostCmdQueue.pop();
          }
        }
        });
    }
}