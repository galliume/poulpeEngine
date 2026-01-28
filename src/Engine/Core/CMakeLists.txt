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
        FreeType.cppm
        FontTypes.cppm
        GLFW.cppm
        JoinThreads.cppm
        Json.cppm
        LightTypes.cppm
        LockFreeStack.cppm
        Logger.cppm
        Lua.cppm
        MaterialTypes.cppm
        GLM.cppm
        ktx.cppm
        MeshTypes.cppm
        MiniAudio.cppm
        PlpTypedef.cppm
        StbImage.cppm
        TclTk.cppm
        ThreadPool.cppm
        ThreadSafeQueue.cppm
        Tools.cppm
        Vertex.cppm
        Volk.cppm
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
    assimp::assimp
    freetype
    glfw
    ktx
    lua
    miniaudio
    nlohmann_json
    stb
    tcl
    tk
    volk)

set_poulpe_definitions(Core)
set_poulpe_links(Core)
set_poulpe_compiler_flags(Core)