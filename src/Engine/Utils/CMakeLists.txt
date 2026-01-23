add_library(Utils STATIC)

target_sources(Utils
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
      FILES
        IDHelper.cppm
        LuaScript.cppm
        ScopedTimer.cppm
  PRIVATE
    LuaScript.cpp)

target_link_libraries(
  Utils

  PRIVATE
    Core
    EngineCommon

  PUBLIC
    lua)

set_poulpe_definitions(Utils)
set_poulpe_links(Utils)
set_poulpe_compiler_flags(Utils)