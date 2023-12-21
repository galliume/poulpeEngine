#pragma once

#include "CommandQueue.hpp"
#include "ThreadPool.hpp"

#include "Poulpe/Manager/InputManager.hpp"

namespace Poulpe {

    class Locator
    {
    public:
        static CommandQueue* getCommandQueue() { return m_CommandQueue.get(); }
        static InputManager* getInputManager() { return m_InputManager.get(); }
        static ThreadPool* getThreadPool() { return m_ThreadPool.get(); }
        
        static void setCommandQueue(CommandQueue* commandQueue) {
          m_CommandQueue = std::unique_ptr<CommandQueue>(commandQueue);}

        static void setInputManager(InputManager* inputManager) {
          m_InputManager = std::unique_ptr<InputManager>(inputManager); }

        //@todo inject with Interface not direct Impl
        static void setThreadPool(ThreadPool* threadPool) {
          m_ThreadPool = std::unique_ptr<ThreadPool>(threadPool); }

    private:
        inline static std::unique_ptr<CommandQueue> m_CommandQueue;
        inline static std::unique_ptr<InputManager> m_InputManager;
        inline static std::unique_ptr<ThreadPool> m_ThreadPool;
    };
}