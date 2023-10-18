#include "CommandQueue.hpp"

namespace Poulpe
{
    CommandQueue::CommandQueue()
    {

    }

    void CommandQueue::Add(Command& cmd)
    {
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            std::shared_ptr<Command> cmdToQueue(std::make_shared<Command>(std::move(cmd)));

            if (cmd.GetWhenToExecute() == WhenToExecute::PRE_RENDERING) m_PreCmdQueue.push(cmdToQueue);
            else if (cmd.GetWhenToExecute() == WhenToExecute::POST_RENDERING) m_PostCmdQueue.push(cmdToQueue);
        }
    }

    void CommandQueue::ExecPreRequest()
    {
        Poulpe::Locator::getThreadPool()->Submit("preCommandQueue", [=, this]() {
            {
                std::lock_guard guard(m_Mutex);

                while (!m_PreCmdQueue.empty())
                {
                    std::shared_ptr<Command> cmd = m_PreCmdQueue.front();
                    cmd->ExecRequest();
                    m_PreCmdQueue.pop();
                }
            }
        });
    }

    void CommandQueue::ExecPostRequest()
    {
      Poulpe::Locator::getThreadPool()->Submit("postCommandQueue", [=, this]() {
        {
          std::lock_guard guard(m_Mutex);

          while (!m_PostCmdQueue.empty())
          {
            std::shared_ptr<Command> cmd = m_PostCmdQueue.front();
            cmd->ExecRequest();
            m_PostCmdQueue.pop();
          }
        }
        });
    }
}