add_library(Core STATIC)

target_sources(Core
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
      FILES
        AnimationTypes.cppm
        AssimpLoader.cppm
        BoneTypes.cppm
        Camera.cppm
        Command.cppm
        CommandQueue.cppm
        CommandQueueLocator.cppm
        Constants.cppm
        FontTypes.cppm
        JoinThreads.cppm
        LightTypes.cppm
        LockFreeStack.cppm
        Logger.cppm
        MaterialTypes.cppm
        MeshTypes.cppm
        PlpTypedef.cppm
        ThreadPool.cppm
        ThreadSafeQueue.cppm
        Tools.cppm
        Vertex.cppm
        VulkanTypes.cppm
  PRIVATE
    AssimpLoader.cpp
    Camera.cpp
    Command.cpp
    CommandQueue.cpp
    CommandQueueLocator.cpp
    Tools.cpp)

target_link_libraries(
  Core

  PRIVATE
    EngineCommon

  PUBLIC
    assimp::assimp)

set_poulpe_definitions(Core)
set_poulpe_links(Core)
set_poulpe_compiler_flags(Core)