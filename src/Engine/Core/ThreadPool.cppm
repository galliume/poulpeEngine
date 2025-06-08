module;

#include <functional>
#include <string_view>
#include <thread>
#include <atomic>

export module Engine.Core.ThreadPool;

import Engine.Core.JoinThreads;
import Engine.Core.LockFreeStack;
import Engine.Core.ThreadSafeQueue;

namespace Poulpe
{
  export class ThreadPool
  {
  public:

    ThreadPool() : _done(false), _joiner(_threads)
    {
      unsigned const threadCount = std::thread::hardware_concurrency();

      try {
        for (unsigned i{ 0 }; i < threadCount; ++i) {
          _threads.emplace_back(std::thread(&ThreadPool::WorkerThreads, this));
        }
      }
      catch (...) {
        _done = true;

        throw;
      }
    }

    template<typename FunctionType>
    void submit(FunctionType f)
    {
      _workQueue.push(std::function<void()>(f));
    }

    bool isPoolEmpty()
    {
      //return (_WorkQueue.contains(poolName)) ? _WorkQueue[poolName].empty() : true;
      return false;
    }

    ~ThreadPool()
    {
      _done = true;
    }

  private:
    void WorkerThreads()
    {
      while (!_done) {
        //@todo add priority order
        auto task{ _workQueue.pop() };

        if (task) {
          (*task.get())();
        } else {
          std::this_thread::yield();
        }
      }
    }

  private:
    std::atomic_bool _done;
    //std::unordered_map<std::string_view, ThreadSafeQueue<std::function<void()>>> _WorkQueue;
    LockFreeStack<std::function<void()>> _workQueue;
    std::vector<std::thread> _threads;
    JoinThreads _joiner;
  };
}
