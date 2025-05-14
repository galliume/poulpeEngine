target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
      FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/Utils.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/IDHelper.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/LuaScript.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/ScopedTimer.cppm)