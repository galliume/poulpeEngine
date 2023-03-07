#pragma once
#include "ThreadPool.hpp"

namespace Rbk{

    class Locator
    {
    public:
        static ThreadPool* getThreadPool() { return m_ThreadPool.get(); }
        static void setThreadPool(std::unique_ptr<ThreadPool> threadPool) { m_ThreadPool = std::move(threadPool); }
    private:
        inline static std::unique_ptr<ThreadPool> m_ThreadPool;

    };
}