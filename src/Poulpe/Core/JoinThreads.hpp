#pragma once
#include <vector>
#include <thread>

namespace Poulpe
{
    class joinThreads
    {
    public:
        explicit joinThreads(std::vector<std::thread>& threads) : m_Threads(threads) {};
        ~joinThreads() {
          for (unsigned long i{ 0 }; i < m_Threads.size(); ++i) {
                if (m_Threads[i].joinable()) {
                    m_Threads[i].join();
                }
            }
        }
    private:
        std::vector<std::thread>& m_Threads;
    };
}