#include "rebulkpch.h"
#include "ThreadPool.h"

namespace Rbk
{
    void ThreadPool::Start()
    {
        const uint32_t numThreads = 4;// std::thread::hardware_concurrency();
        m_Threads.resize(numThreads);

        for (uint32_t i = 0; i < numThreads; i++) {
            m_Threads.at(i) = std::thread(&ThreadPool::Loop, this);
        }
    }

    void ThreadPool::Loop()
    {
        while (true) {
            std::function<void()> job;
            {
                std::unique_lock<std::mutex> lock(m_QueueMutex);

                m_MutexCondition.wait(lock, [this] { return !m_Jobs.empty() || m_ShouldTerminate; });

                if (m_ShouldTerminate) {
                    return;
                }

                job = m_Jobs.front();
                m_Jobs.pop();
            }

            job();
        }
    }

    void ThreadPool::Queue(const std::function<void()>& job)
    {
        {
            std::unique_lock<std::mutex> lock(m_QueueMutex);
            m_Jobs.push(job);
        }

        m_MutexCondition.notify_one();
    }

    bool ThreadPool::Busy()
    {
        bool poolbusy;
        {
            std::unique_lock<std::mutex> lock(m_QueueMutex);
            poolbusy = m_Jobs.empty();
        }
        return poolbusy;
    }

    void ThreadPool::Stop()
    {
        {
            std::unique_lock<std::mutex> lock(m_QueueMutex);
            m_ShouldTerminate = true;
        }
        
        m_MutexCondition.notify_all();

        for (std::thread& activeThread : m_Threads) {
            activeThread.join();
        }
        m_Threads.clear();
    }
}
