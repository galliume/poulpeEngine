#set(SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/Application.cpp
#             ${CMAKE_CURRENT_SOURCE_DIR}/Application.hpp)

#target_sources(PoulpeEngine PRIVATE ${SRC_LIST})

target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET cxx_headers TYPE HEADERS
      FILES
        ${PROJECT_SOURCE_DIR}/src/Poulpe/Application.hpp
  PRIVATE
    ${PROJECT_SOURCE_DIR}/src/Poulpe/Application.cpp)