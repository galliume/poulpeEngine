#include "Command.hpp"

namespace Poulpe
{
    Command::Command(std::function<void()> request, WhenToExecute w)
        : m_Request(request), m_When(w)
    {
    }

    Response Command::ExecRequest()
    {
        Response response;
        
        m_Request();

        return response;
    }
}