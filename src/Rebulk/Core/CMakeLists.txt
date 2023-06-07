set(SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/Buffer.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/IVisitor.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/IVisitor.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/JoinThreads.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Locator.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Log.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Log.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/MeshData.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/ThreadPool.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/ThreadSafeQueue.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/TinyObjLoader.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/TinyObjLoader.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Tools.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Tools.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Command.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Command.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/CommandQueue.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/CommandQueue.hpp)

target_sources(Rebulkan PRIVATE ${SRC_LIST})
