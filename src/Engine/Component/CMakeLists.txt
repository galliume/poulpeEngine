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

  PUBLIC
    fmt::fmt
    freetype
    glm
    volk)

set_poulpe_definitions(Components)
set_poulpe_links(Components)