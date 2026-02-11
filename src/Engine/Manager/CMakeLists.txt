add_library(EngineManagers STATIC)

target_sources(EngineManagers
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
      FILES
        APIManager.cppm
        AudioManager.cppm
        ComponentManager.cppm
        ConfigManager.cppm
        ConfigManagerLocator.cppm
        DbManager.cppm
        DestroyManager.cppm
        EntityManager.cppm
        FontManager.cppm
        InputManager.cppm
        InputManagerLocator.cppm
        LightManager.cppm
        NetworkManager.cppm
        PlayerManager.cppm
        RenderManager.cppm
        ShaderManager.cppm
        SystemManager.cppm
        TextureManager.cppm
        UIManager.cppm

  PRIVATE
    APIManager.cpp
    AudioManager.cpp
    ComponentManager.cpp
    ConfigManager.cpp
    ConfigManagerLocator.cpp
    DbManager.cpp
    DestroyManager.cpp
    EntityManager.cpp
    FontManager.cpp
    InputManager.cpp
    InputManagerLocator.cpp
    LightManager.cpp
    NetworkManager.cpp
    PlayerManager.cpp
    RenderManager.cpp
    ShaderManager.cpp
    SystemManager.cpp
    TextureManager.cpp
    UIManager.cpp)

target_link_libraries(
  EngineManagers

  PRIVATE
    Animation
    Components
    Core
    EngineCommon
    GUI
    Network
    Renderer

  PUBLIC
    glfw
    ktx
    miniaudio
    nlohmann_json
    stb
    sqlite3)

set_poulpe_definitions(EngineManagers)
set_poulpe_links(EngineManagers)
set_poulpe_compiler_flags(EngineManagers)