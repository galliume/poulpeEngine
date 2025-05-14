target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
      FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/Network.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/WinServer.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/WinSocket.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/UnixServer.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/UnixSocket.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/Socket.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/Server.cppm
  PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/WinServer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/WinSocket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UnixServer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UnixSocket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Socket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp)