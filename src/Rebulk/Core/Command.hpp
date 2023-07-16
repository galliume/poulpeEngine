#pragma once

namespace Rbk
{
    struct Response
    {

    };

    class Command
    {
    public:
        Command(std::function<void()> request);
        Response ExecRequest();

    private:
        std::function<void()> m_Request;
    };
}