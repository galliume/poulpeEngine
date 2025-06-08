#set(SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/Window.cppm)

#target_sources(PoulpeEngine PRIVATE ${SRC_LIST})

target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
      FILES
        Window.cppm
  PRIVATE
    Window.cpp)