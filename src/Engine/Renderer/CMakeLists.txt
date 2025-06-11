source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_LIST})

target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
      FILES
        Mesh.cppm
        Renderers.cppm
        Renderer.cppm
        RendererComponentTypes.cppm
        RendererComponent.cppm
        RendererComponentFactory.cppm)
