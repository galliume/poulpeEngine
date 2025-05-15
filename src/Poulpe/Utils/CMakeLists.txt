target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
      FILES
        IDHelper.cppm
        LuaScript.cppm
        ScopedTimer.cppm
  PRIVATE
    LuaScript.cpp)