#include "CommandQueue.hpp"

namespace Poulpe
{
  CommandQueue::CommandQueue()
  {

  }

  void CommandQueue::add(Command& cmd)
  {
    {
      std::lock_guard<std::mutex> lock(_Mutex);
      std::shared_ptr<Command> cmdToQueue(std::make_shared<Command>(std::move(cmd)));

      if (cmd.getWhenToExecute() == WhenToExecute::PRE_RENDERING) _PreCmdQueue.push(cmdToQueue);
      else if (cmd.getWhenToExecute() == WhenToExecute::POST_RENDERING) _PostCmdQueue.push(cmdToQueue);
    }
  }

  void CommandQueue::execPostRequest()
  {
    if (_PostCmdQueue.empty()) return;

    std::jthread command([this]() {
      {
        std::lock_guard guard(_Mutex);

        while (!_PostCmdQueue.empty())
        {
          std::shared_ptr<Command> cmd = _PostCmdQueue.front();
          cmd->execRequest();
          _PostCmdQueue.pop();
        }
      }
    });
    command.detach();
  }

  void CommandQueue::execPreRequest()
  {
    if (_PreCmdQueue.empty()) return;

    std::jthread command([this]() {
      {
        std::lock_guard guard(_Mutex);

        while (!_PreCmdQueue.empty())
        {
          std::shared_ptr<Command> cmd = _PreCmdQueue.front();
          cmd->execRequest();
          _PreCmdQueue.pop();
        }
      }
    });
    command.detach();
  }
}
