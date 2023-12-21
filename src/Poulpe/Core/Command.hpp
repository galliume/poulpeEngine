#pragma once

namespace Poulpe
{
    enum class WhenToExecute {PRE_RENDERING, POST_RENDERING};

    struct Response
    {

    };

    class Command
    {
    public:
        Command(std::function<void()> request, WhenToExecute w = WhenToExecute::PRE_RENDERING);
        
        Response execRequest();
        WhenToExecute getWhenToExecute() const { return m_When; };

    private:
        std::function<void()> m_Request;
        WhenToExecute m_When;
    };
}