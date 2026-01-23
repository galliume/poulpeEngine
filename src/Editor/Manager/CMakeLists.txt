add_library(EditorManagers STATIC)

target_sources(EditorManagers
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
      FILES
        EditorManager.cppm
        LevelManager.cppm
        ExplorerManager.cppm

  PRIVATE
    EditorManager.cpp
    LevelManager.cpp
    ExplorerManager.cpp)

target_link_libraries(
  EditorManagers 

  PRIVATE
    Components
    EngineManagers
    PoulpeEngineLib

  PUBLIC
    fmt::fmt
    glfw
    tcl
    tk)

set_poulpe_definitions(EditorManagers)
set_poulpe_links(EditorManagers)