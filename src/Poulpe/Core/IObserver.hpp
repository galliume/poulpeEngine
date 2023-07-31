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
        virtual void Notify(const Event& onEvent) = 0;
    };
}