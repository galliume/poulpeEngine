target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
      BASE_DIRS ${PROJECT_SOURCE_DIR}/src/Poulpe
      FILES
        Basic.cppm
        Crosshair.cppm
        Grid.cppm
        RendererFactory.cppm
        Skybox.cppm
        ShadowMap.cppm
        Terrain.cppm
        Text.cppm
        Water.cppm
  PRIVATE
    Basic.cpp
    Crosshair.cpp
    Grid.cpp
    Skybox.cpp
    ShadowMap.cpp
    Terrain.cpp
    Text.cpp
    Water.cpp)