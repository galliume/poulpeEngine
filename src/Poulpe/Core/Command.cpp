module Poulpe.Core.Command;

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
