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

    ThreadPool() : m_Done(false), m_Joiner(m_Threads)
    {
      unsigned const threadCount = std::thread::hardware_concurrency();

      try {
        for (unsigned i{ 0 }; i < threadCount; ++i) {
          m_Threads.emplace_back(std::thread(&ThreadPool::WorkerThreads, this));
        }
      }
      catch (...) {
        m_Done = true;

        throw;
      }
    }

    template<typename FunctionType>
    void submit(std::string_view queueName, FunctionType f)
    {
      auto& workEntry = m_WorkQueue[queueName];
      workEntry.push(std::function<void()>(f));
    }

    bool isPoolEmpty(std::string_view poolName)
    {
      //return (m_WorkQueue.contains(poolName)) ? m_WorkQueue[poolName].empty() : true;
      return false;
    }

    ~ThreadPool()
    {
      m_Done = true;
    }

  private:
    void WorkerThreads()
    {
      while (!m_Done) {
        //std::function<void()> task;
        //@todo add priority order
        std::ranges::for_each(m_WorkQueue, [](auto &pair) {
            //if (!queueThread.empty() && queueThread.tryPop(task)) {
            //    task();
            //} else {
            //    std::this_thread::yield();
            //}
          auto& [queueName, queueThread] = pair;
          std::unique_ptr<std::function<void()>> task = queueThread.pop();

          if (task) {
            (*task.get())();
          } else {
            std::this_thread::yield();
          }
        });
      }
    }

  private:
      std::atomic_bool m_Done;
      //std::unordered_map<std::string_view, ThreadSafeQueue<std::function<void()>>> m_WorkQueue;
      std::unordered_map<std::string_view, LockFreeStack<std::function<void()>>> m_WorkQueue;
      std::vector<std::thread> m_Threads;
      joinThreads m_Joiner;
  };
}