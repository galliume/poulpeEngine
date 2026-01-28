add_library(GUI STATIC)

target_sources(GUI
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
      FILES
        Window.cppm
  PRIVATE
    Window.cpp)

target_link_libraries(
  GUI

  PRIVATE
    Core
    EngineCommon

  PUBLIC
    glfw
    stb)

set_poulpe_definitions(GUI)
set_poulpe_links(GUI)
set_poulpe_compiler_flags(GUI)