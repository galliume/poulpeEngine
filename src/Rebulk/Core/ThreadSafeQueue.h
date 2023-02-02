#pragma once
#include <queue>

namespace Rbk
{
    template<typename T>
    class ThreadSafeQueue
    {
    public:
        ThreadSafeQueue() {};

        void Push(T newValue)
        {
            std::shared_ptr<T>data(std::make_shared<T>(std::move(newValue)));
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_DataQueue.push(data);
            m_DataCond.notify_one();
        }

        void WaitAndPop(T& value)
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_DataCond.wait(lock, [this] { return !m_DataQueue.empty(); });

            value = std::move(*m_DataQueue.front());
            m_DataQueue.pop();
        }

        std::shared_ptr<T> WaitAndPop()
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_DataCond.wait(lock, [this] { return !m_DataQueue.empty(); });

            std::shared_ptr<T> res = m_DataQueue.front();
            m_DataQueue.pop();

            return res;
        }

        bool TryPop(T& value)
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            if (m_DataQueue.empty()) return false;

            value = std::move(*m_DataQueue.front());
            m_DataQueue.pop();

            return true;
        }

        std::shared_ptr<T> TryPop()
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            if (m_DataQueue.empty()) return false;

            std::shared_ptr<T> res = m_DataQueue.front();
            m_DataQueue.pop();

            return res;
        }

        bool Empty()
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return m_DataQueue.empty();
        }

    private:
        mutable std::mutex m_Mutex;
        std::queue<std::shared_ptr<T>> m_DataQueue;
        std::condition_variable m_DataCond;
    };
}