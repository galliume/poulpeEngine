#pragma once
#include <vector>
#include <thread>

namespace Poulpe
{
    class JoinThreads
    {
    public:
        explicit JoinThreads(std::vector<std::thread>& threads) : m_Threads(threads) {};
        ~JoinThreads() {
            for (unsigned long i = 0; i < m_Threads.size(); ++i) {
                if (m_Threads[i].joinable()) {
                    m_Threads[i].join();
                }
            }
        }
    private:
        std::vector<std::thread>& m_Threads;
    };
}