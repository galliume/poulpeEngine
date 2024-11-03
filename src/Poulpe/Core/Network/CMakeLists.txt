set(SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/WinServer.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/WinServer.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/WinSocket.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/WinSocket.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/UnixServer.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/UnixServer.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/UnixSocket.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/UnixSocket.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Socket.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Socket.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Server.hpp)

target_sources(PoulpeEngine PRIVATE ${SRC_LIST})
