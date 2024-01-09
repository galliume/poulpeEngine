#include "Command.hpp"

namespace Poulpe
{
    Command::Command(std::function<void()> request, WhenToExecute w)
        : m_Request(request), m_When(w)
    {
    }

    //@todo WIP
    Response Command::execRequest()
    {
        Response response;
        
        m_Request();

        return response;
    }
}
