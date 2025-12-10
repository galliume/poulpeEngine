export module Engine.Core.ThreadSafeQueue;

import std;

namespace Poulpe
{
  export template<typename T>
  class ThreadSafeQueue
  {
  public:
    ThreadSafeQueue() {}
    ~ThreadSafeQueue() = default;

    void push(T newValue)
    {
      {
        std::lock_guard<std::mutex> lock(_mutex);
        std::shared_ptr<T>data(std::make_shared<T>(std::move(newValue)));
        _dataQueue.push(data);
        _dataCond.notify_one();
      }
    }

    void waitAndPop(T& value)
    {
        {
        std::unique_lock<std::mutex> lock(_mutex);
        _dataCond.wait(lock, [=, this] { return !_dataQueue.empty(); });

        value = std::move(*_dataQueue.front());
        _dataQueue.pop();
      }
    }

    std::shared_ptr<T> waitAndPop()
    {
      {
        std::unique_lock<std::mutex> lock(_mutex);
        _dataCond.wait(lock, [=, this] { return !_dataQueue.empty(); });

        std::shared_ptr<T> res = _dataQueue.front();
        _dataQueue.pop();
        return res;
      }
    }

    bool tryPop(T& value)
    {
      {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_dataQueue.empty()) return false;

        value = std::move(*_dataQueue.front());
        _dataQueue.pop();
      }

      return true;
    }

    std::shared_ptr<T> tryPop()
    {
      {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_dataQueue.empty()) return false;

        std::shared_ptr<T> res = _dataQueue.front();
        _dataQueue.pop();
        return res;
      }
    }

    bool empty()
    {
      bool isEmpty{ false };

      {
        std::lock_guard<std::mutex> lock(_mutex);
        isEmpty = _dataQueue.empty();
      }

      return isEmpty;
    }

  private:
      mutable std::mutex _mutex;
      std::queue<std::shared_ptr<T>> _dataQueue;
      std::condition_variable _dataCond;
  };
}
