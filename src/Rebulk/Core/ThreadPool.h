#pragma once
#include <queue>

namespace Rbk
{
    class ThreadPool
    {
        public:
            void Start();
            void Queue(const std::function<void()>& job);
            void Stop();
            bool Busy();

        private:
            void Loop();

            bool m_ShouldTerminate = false;
            std::mutex m_QueueMutex;
            std::condition_variable m_MutexCondition;
            std::vector<std::thread> m_Threads;
            std::queue<std::function<void()>> m_Jobs;
    };
}
