#pragma once
#include <queue>

namespace Rbk
{
    template<typename T>
    class ThreadSafeQueue
    {
    public:
        ThreadSafeQueue() {};
        ~ThreadSafeQueue() = default;

        void Push(T newValue)
        {
            {
                std::lock_guard<std::mutex> lock(m_PushMutex);
                std::shared_ptr<T>data(std::make_shared<T>(std::move(newValue)));
                m_DataQueue.push(data);
                m_DataCond.notify_one();
            }
        }

        void WaitAndPop(T& value)
        {
            {
                std::unique_lock<std::mutex> lock(m_WaitMutex);
                m_DataCond.wait(lock, [=, this] { return !m_DataQueue.empty(); });

                value = std::move(*m_DataQueue.front());
                m_DataQueue.pop();
            }
        }

        std::shared_ptr<T> WaitAndPop()
        {
            {
                std::unique_lock<std::mutex> lock(m_WaitMutex);
                m_DataCond.wait(lock, [=, this] { return !m_DataQueue.empty(); });

                std::shared_ptr<T> res = m_DataQueue.front();
                m_DataQueue.pop();
            }

            return res;
        }

        bool TryPop(T& value)
        {
            {
                std::lock_guard<std::mutex> lock(m_PopMutex);
                if (m_DataQueue.empty()) return false;

                value = std::move(*m_DataQueue.front());
                m_DataQueue.pop();
            }

            return true;
        }

        std::shared_ptr<T> TryPop()
        {
            {
                std::lock_guard<std::mutex> lock(m_PopMutex);
                if (m_DataQueue.empty()) return false;

                std::shared_ptr<T> res = m_DataQueue.front();
                m_DataQueue.pop();
            }

            return res;
        }

        bool Empty()
        {
            bool isEmpty{ false };

            {
                std::lock_guard<std::mutex> lock(m_Mutex);
                isEmpty = m_DataQueue.empty();
            }

            return isEmpty;
        }

    private:
        mutable std::mutex m_Mutex;
        mutable std::mutex m_PushMutex;
        mutable std::mutex m_WaitMutex;
        mutable std::mutex m_PopMutex;
        std::queue<std::shared_ptr<T>> m_DataQueue;
        std::condition_variable m_DataCond;
    };
}