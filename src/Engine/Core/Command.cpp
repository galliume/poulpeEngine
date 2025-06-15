module;
#include <functional>

module Engine.Core.Command;

namespace Poulpe
{
  Command::Command(
    std::function<void()> request
    , WhenToExecute w)
    : _request(request), _when(w)
  {
  }

  //@todo WIP
  Response Command::execRequest()
  {
    Response response;

    _request();

    return response;
  }

  WhenToExecute Command::getWhenToExecute() const 
  {
    return _when;
  }
}
