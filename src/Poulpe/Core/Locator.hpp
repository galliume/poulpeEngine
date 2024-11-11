#pragma once

#include "CommandQueue.hpp"
#include "ThreadPool.hpp"

#include "Poulpe/Manager/InputManager.hpp"
#include "Poulpe/Manager/ConfigManager.hpp"

namespace Poulpe {

    class Locator
    {
    public:
        static CommandQueue* getCommandQueue() { return _CommandQueue.get(); }
        static InputManager* getInputManager() { return _InputManager.get(); }
        static ConfigManager* getConfigManager() { return _ConfigManager.get(); }
        //static ThreadPool* getThreadPool() { return _ThreadPool.get(); }
        
        static void init(Window const * const window)
        {
          _InputManager = std::make_unique<InputManager>(window);
          _CommandQueue = std::make_unique<CommandQueue>();
          _ConfigManager = std::make_unique<ConfigManager>();
          //_ThreadPool = std::make_unique<ThreadPool>();
        }

    private:
        inline static std::unique_ptr<CommandQueue> _CommandQueue;
        inline static std::unique_ptr<ConfigManager> _ConfigManager;
        inline static std::unique_ptr<InputManager> _InputManager;
        //inline static std::unique_ptr<ThreadPool> _ThreadPool;
    };
}