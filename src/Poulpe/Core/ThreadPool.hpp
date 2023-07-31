#pragma once

#include <functional>

#include "ThreadSafeQueue.hpp"
#include "JoinThreads.hpp"

namespace Poulpe
{
    class ThreadPool
    {
    public:
    
        ThreadPool() : m_Done(false), m_Joiner(m_Threads)
        {
            unsigned const threadCount = 5;//std::thread::hardware_concurrency();

            try {
                for (unsigned i = 0; i < threadCount; ++i) {
                    m_Threads.push_back(std::thread(&ThreadPool::WorkerThreads, this));
                }
            }
            catch (...) {
                m_Done = true;

                throw;
            }
        }

        template<typename FunctionType>
        void Submit(std::string_view queueName, FunctionType f)
        {
            if (m_WorkQueue.contains(queueName)) {
                m_WorkQueue.at(queueName).Push(std::function<void()>(f));
            } else{
                m_WorkQueue[queueName];
                m_WorkQueue.at(queueName).Push(std::function<void()>(f));
            }
        }

        bool IsPoolEmpty(std::string_view poolName)
        {
            return (m_WorkQueue.contains(poolName)) ? m_WorkQueue[poolName].Empty() : true;
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
                //@todo add priority order
                for (auto& [queueName, queueThread]: m_WorkQueue) {
                    if (queueThread.TryPop(task)) {
                        task();
                    } else {
                        std::this_thread::yield();
                    }
                }
            }
        }

    private:
        std::atomic_bool m_Done;
        std::unordered_map<std::string_view, ThreadSafeQueue<std::function<void()>>> m_WorkQueue;
        std::vector<std::thread> m_Threads;
        JoinThreads m_Joiner;
    };
}