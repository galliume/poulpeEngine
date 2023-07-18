#pragma once

namespace Rbk
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