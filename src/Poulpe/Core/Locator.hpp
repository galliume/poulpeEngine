#pragma once

#include "Poulpe/Manager/InputManager.hpp"
#include "ThreadPool.hpp"

namespace Poulpe {

    class Locator
    {
    public:
        static ThreadPool* getThreadPool() { return m_ThreadPool.get(); }
        static InputManager* getInputManager() { return m_InputManager.get(); }

        static void setThreadPool(std::unique_ptr<ThreadPool> threadPool) { m_ThreadPool = std::move(threadPool); }
        static void setInputManager(std::shared_ptr<InputManager> inputManager) { m_InputManager = inputManager; }

    private:
        inline static std::unique_ptr<ThreadPool> m_ThreadPool;
        inline static std::shared_ptr<InputManager> m_InputManager;

    };
}