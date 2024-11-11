#pragma once

#include "JoinThreads.hpp"
#include "ThreadSafeQueue.hpp"
#include "LockFreeStack.hpp"

#include <functional>

namespace Poulpe
{
  class ThreadPool
  {
  public:

    ThreadPool() : _Done(false), _Joiner(_Threads)
    {
      unsigned const threadCount = std::thread::hardware_concurrency();

      try {
        for (unsigned i{ 0 }; i < threadCount; ++i) {
          _Threads.emplace_back(std::thread(&ThreadPool::WorkerThreads, this));
        }
      }
      catch (...) {
        _Done = true;

        throw;
      }
    }

    template<typename FunctionType>
    void submit(FunctionType f)
    {
      _WorkQueue.push(std::function<void()>(f));
    }

    bool isPoolEmpty(std::string_view poolName)
    {
      //return (_WorkQueue.contains(poolName)) ? _WorkQueue[poolName].empty() : true;
      return false;
    }

    ~ThreadPool()
    {
      _Done = true;
    }

  private:
    void WorkerThreads()
    {
      while (!_Done) {
        //@todo add priority order
        auto task{ _WorkQueue.pop() };

        if (task) {
          (*task.get())();
        } else {
          std::this_thread::yield();
        }
      }
    }

  private:
      std::atomic_bool _Done;
      //std::unordered_map<std::string_view, ThreadSafeQueue<std::function<void()>>> _WorkQueue;
      LockFreeStack<std::function<void()>> _WorkQueue;
      std::vector<std::thread> _Threads;
      joinThreads _Joiner;
  };
}