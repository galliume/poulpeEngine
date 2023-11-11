#pragma once

#include "CommandQueue.hpp"
#include "ThreadPool.hpp"

#include "Poulpe/Manager/InputManager.hpp"

namespace Poulpe {

    class Locator
    {
    public:
        static ThreadPool* getThreadPool() { return m_ThreadPool.get(); }
        static InputManager* getInputManager() { return m_InputManager.get(); }
        static CommandQueue* getCommandQueue() { return m_CommandQueue.get(); }

        static void setThreadPool(std::unique_ptr<ThreadPool> threadPool) { m_ThreadPool = std::move(threadPool); }
        static void setInputManager(std::shared_ptr<InputManager> inputManager) { m_InputManager = std::move(inputManager); }
        static void setCommandQueue(std::shared_ptr<CommandQueue> commandQueue) { m_CommandQueue = std::move(commandQueue); }

    private:
        inline static std::unique_ptr<ThreadPool> m_ThreadPool;
        inline static std::shared_ptr<InputManager> m_InputManager;
        inline static std::shared_ptr<CommandQueue> m_CommandQueue;
    };
}