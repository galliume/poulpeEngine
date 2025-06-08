module;
#include <queue>
#include <memory>
#include <mutex>
#include <thread>

module Engine.Core.CommandQueue;

namespace Poulpe
{
  // void CommandQueue::add(Command& cmd)
  // {
  //   {
  //     std::lock_guard<std::mutex> lock(_mutex);
  //     std::shared_ptr<Command> cmdToQueue(std::make_shared<Command>(std::move(cmd)));

  //     if (cmd.getWhenToExecute() == WhenToExecute::PRE_RENDERING) _preCmdQueue.insert(_preCmdQueue.begin(), cmdToQueue);
  //     else if (cmd.getWhenToExecute() == WhenToExecute::POST_RENDERING) _postCmdQueue.insert(_preCmdQueue.begin(), cmdToQueue);
  //   }
  // }

  // void CommandQueue::execPostRequest()
  // {
  //   if (_postCmdQueue.empty()) return;

  //   std::jthread command([this]() {
  //     {
  //       std::lock_guard guard(_mutex);

  //       while (!_postCmdQueue.empty())
  //       {
  //         std::shared_ptr<Command> cmd = _postCmdQueue.back();
  //         cmd->execRequest();
  //         _postCmdQueue.pop_back();
  //       }
  //     }
  //   });
  //   command.detach();
  // }

  // void CommandQueue::execPreRequest()
  // {
  //   if (_preCmdQueue.empty()) return;

  //   std::jthread command([this]() {
  //     {
  //       std::lock_guard guard(_mutex);

  //       while (!_preCmdQueue.empty())
  //       {
  //         std::shared_ptr<Command> cmd = _preCmdQueue.back();
  //         cmd->execRequest();
  //         _preCmdQueue.pop_back();
  //       }
  //     }
  //   });
  //   command.detach();
  // }
}
