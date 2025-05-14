export module Poulpe.Core:ThreadSafeQueue;

import <condition_variable>;
import <queue>;

template<typename T>
export class ThreadSafeQueue
{
public:
    ThreadSafeQueue() {};
    ~ThreadSafeQueue() = default;

    void push(T newValue)
    {
        {
            std::lock_guard<std::mutex> lock(_Mutex);
            std::shared_ptr<T>data(std::make_shared<T>(std::move(newValue)));
            _DataQueue.push(data);
            _DataCond.notify_one();
        }
    }

    void waitAndPop(T& value)
    {
        {
            std::unique_lock<std::mutex> lock(_Mutex);
            _DataCond.wait(lock, [=, this] { return !_DataQueue.empty(); });

            value = std::move(*_DataQueue.front());
            _DataQueue.pop();
        }
    }

    std::shared_ptr<T> waitAndPop()
    {
        {
            std::unique_lock<std::mutex> lock(_Mutex);
            _DataCond.wait(lock, [=, this] { return !_DataQueue.empty(); });

            std::shared_ptr<T> res = _DataQueue.front();
            _DataQueue.pop();
            return res;
        }
    }

    bool tryPop(T& value)
    {
        {
            std::lock_guard<std::mutex> lock(_Mutex);
            if (_DataQueue.empty()) return false;

            value = std::move(*_DataQueue.front());
            _DataQueue.pop();
        }

        return true;
    }

    std::shared_ptr<T> tryPop()
    {
        {
            std::lock_guard<std::mutex> lock(_Mutex);
            if (_DataQueue.empty()) return false;

            std::shared_ptr<T> res = _DataQueue.front();
            _DataQueue.pop();
            return res;
        }
    }

    bool empty()
    {
        bool isEmpty{ false };

        {
            std::lock_guard<std::mutex> lock(_Mutex);
            isEmpty = _DataQueue.empty();
        }

        return isEmpty;
    }

private:
    mutable std::mutex _Mutex;
    std::queue<std::shared_ptr<T>> _DataQueue;
    std::condition_variable _DataCond;
};
