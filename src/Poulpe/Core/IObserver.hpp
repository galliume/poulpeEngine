#pragma once

namespace Poulpe
{
    struct Event
    {
        std::string_view name;
    };

    class IObserver
    {
    public:
        virtual void notify(const Event& onEvent) = 0;
    };
}