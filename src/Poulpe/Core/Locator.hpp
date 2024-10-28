#pragma once

#include "CommandQueue.hpp"
#include "ThreadPool.hpp"

#include "Poulpe/Manager/InputManager.hpp"
#include "Poulpe/Manager/ConfigManager.hpp"

namespace Poulpe {

    class Locator
    {
    public:
        static CommandQueue* getCommandQueue() { return m_CommandQueue.get(); }
        static InputManager* getInputManager() { return m_InputManager.get(); }
        static ConfigManager* getConfigManager() { return m_ConfigManager.get(); }
        //static ThreadPool* getThreadPool() { return m_ThreadPool.get(); }
        
        static void init(Window const * const window)
        {
          m_InputManager = std::make_unique<InputManager>(window);
          m_CommandQueue = std::make_unique<CommandQueue>();
          m_ConfigManager = std::make_unique<ConfigManager>();
          //m_ThreadPool = std::make_unique<ThreadPool>();
        }

    private:
        inline static std::unique_ptr<CommandQueue> m_CommandQueue;
        inline static std::unique_ptr<ConfigManager> m_ConfigManager;
        inline static std::unique_ptr<InputManager> m_InputManager;
        //inline static std::unique_ptr<ThreadPool> m_ThreadPool;
    };
}