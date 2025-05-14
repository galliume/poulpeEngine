module;
#include <functional>

export module Poulpe.Core.Command;

export enum class WhenToExecute {PRE_RENDERING, POST_RENDERING};

export struct Response
{

};

export class Command
{
public:
  Command(std::function<void()> request, WhenToExecute w = WhenToExecute::PRE_RENDERING);

  Response execRequest();
  WhenToExecute getWhenToExecute() const { return _when; }

private:
  std::function<void()> _request;
  WhenToExecute _when;
};
