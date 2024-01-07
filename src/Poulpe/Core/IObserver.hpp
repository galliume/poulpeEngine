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
        virtual ~IObserver() = 0;
        virtual void notify(const Event& onEvent) = 0;
    };
}
