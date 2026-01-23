add_library(Components STATIC)

target_sources(Components
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
      FILES
        AnimationTypes.cppm
        Components.cppm
        Entity.cppm
        EntityNode.cppm
        Mesh.cppm
        Texture.cppm
  PRIVATE
    Entity.cpp
    Mesh.cpp)

target_link_libraries(
  Components

  PRIVATE
    Utils
    EngineCommon

  PUBLIC
    freetype)

set_poulpe_definitions(Components)
set_poulpe_links(Components)
set_poulpe_compiler_flags(Components)