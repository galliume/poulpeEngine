#pragma once
#include <vector>
#include <thread>

namespace Rbk
{
    class JoinThreads
    {
    public:
        explicit JoinThreads(std::vector<std::thread>& threads) : m_Threads(threads) {};
        ~JoinThreads() {
            RBK_DEBUG("DESTROY jointhreads");
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