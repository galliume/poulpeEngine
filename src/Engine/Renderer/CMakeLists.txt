source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_LIST})

add_library(Renderer STATIC)

target_sources(Renderer
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
      FILES
        Renderer.cppm
        RendererComponentFactory.cppm
  PRIVATE)

target_link_libraries(
  Renderer

  PRIVATE
    Utils
    Vulkan

  PUBLIC
    fmt::fmt
    freetype
    glm
    volk)

set_poulpe_definitions(Renderer)
set_poulpe_links(Renderer)