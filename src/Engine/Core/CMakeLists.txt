add_library(Core STATIC)

target_sources(Core
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
      FILES
        AssimpLoader.cppm
        Camera.cppm
        Command.cppm
        CommandQueue.cppm
        CommandQueueLocator.cppm
        JoinThreads.cppm
        LockFreeStack.cppm
        Logger.cppm
        MeshTypes.cppm
        PlpTypedef.cppm
        ThreadPool.cppm
        ThreadSafeQueue.cppm
        Tools.cppm
        Vertex.cppm
  PRIVATE
    AssimpLoader.cpp
    Camera.cpp
    Command.cpp
    CommandQueue.cpp
    CommandQueueLocator.cpp
    Tools.cpp)

target_link_libraries(
  Core

  PUBLIC
    fmt::fmt
    assimp::assimp
    glm
    volk)

set_poulpe_definitions(Core)
set_poulpe_links(Core)