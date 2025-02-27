set(SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/PlpTypedef.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/JoinThreads.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Locator.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Log.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/ThreadPool.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/ThreadSafeQueue.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/AssimpLoader.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/AssimpLoader.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Tools.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Tools.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Command.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Command.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/CommandQueue.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/CommandQueue.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/LockFreeStack.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/PlpArray.hpp)

target_sources(PoulpeEngine PRIVATE ${SRC_LIST})
