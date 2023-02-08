#pragma once

#include <functional>

#include "ThreadSafeQueue.hpp"
#include "JoinThreads.hpp"

namespace Rbk
{
    class ThreadPool
    {
    public:
    
        ThreadPool() : m_Done(false), m_Joiner(m_Threads)
        {
            unsigned const threadCount = std::thread::hardware_concurrency();
            Rbk::Log::GetLogger()->debug("Starting {} threads", threadCount);

            try {
                for (unsigned i = 0; i < threadCount; ++i) {
                    m_Threads.push_back(std::thread(&ThreadPool::WorkerThreads, this));
                }
            }
            catch (...) {
                m_Done = true;
                Rbk::Log::GetLogger()->critical("ThreadPool error");

                throw;
            }
        }

        template<typename FunctionType>
        void Submit(FunctionType f)
        {
            m_WorkQueue.Push(std::function<void()>(f));
        }

        ~ThreadPool()
        {
            m_Done = true;
        }

    private:
        void WorkerThreads()
        {
            while (!m_Done) {
                std::function<void()> task;

                if (m_WorkQueue.TryPop(task)) {
                    task();
                } else {
                    std::this_thread::yield();
                }
            }
        }

    private:
        std::atomic_bool m_Done;
        ThreadSafeQueue<std::function<void()>> m_WorkQueue;
        std::vector<std::thread> m_Threads;
        JoinThreads m_Joiner;
    };
}