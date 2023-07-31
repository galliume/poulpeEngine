#include "Command.hpp"

namespace Poulpe
{
    Command::Command(std::function<void()> request)
        : m_Request(request)
    {
    }

    Response Command::ExecRequest()
    {
        Response response;
        
        m_Request();

        return response;
    }
}