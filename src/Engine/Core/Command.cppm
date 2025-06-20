module;
#include <functional>
#include <cstdint>

export module Engine.Core.Command;

namespace Poulpe
{
  export enum class WhenToExecute : uint8_t
  {
    PRE_RENDERING,
    POST_RENDERING
  };

  export struct Response
  {

  };

  export class Command
  {
  public:
    Command(std::function<void()> request, WhenToExecute w = WhenToExecute::PRE_RENDERING);

    Response execRequest();
    WhenToExecute getWhenToExecute() const;

  private:
    std::function<void()> _request;
    WhenToExecute _when;
  };
}
