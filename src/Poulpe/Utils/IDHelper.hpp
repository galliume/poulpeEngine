#pragma once

#include <chrono>
#include <cstdint>
#include <random>

namespace Poulpe
{
    using IDType = std::uint64_t;
    using ComponentTypeID = IDType;

    //Unique ID for Entity
    class GUIDGenerator
    {
    public:
        static IDType getGUID()
        {
            auto millis = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch()
            ).count());

            std::random_device rd;
            std::mt19937_64 gen(rd());
            std::uniform_int_distribution<uint64_t> dis(0, std::numeric_limits<uint64_t>::max());
            std::uint64_t random_number = dis(gen);

            return millis + random_number;
        }
    };

    //Unique ID for Archetype
    template<class T>
    class TypeIdGenerator
    {
    private:

        static IDType m_count;

    public:

        template<class U>
        static IDType GetNewID()
        {
            static const IDType idCounter = m_count++;
            return idCounter;
        }
    };

    template<class T> IDType TypeIdGenerator<T>::m_count = 0;
}