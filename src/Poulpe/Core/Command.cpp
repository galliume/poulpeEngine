#include "Command.hpp"

namespace Poulpe
{
    Command::Command(std::function<void()> request, WhenToExecute w)
        : _Request(request), _When(w)
    {
    }

    //@todo WIP
    Response Command::execRequest()
    {
        Response response;
        
        _Request();

        return response;
    }
}
