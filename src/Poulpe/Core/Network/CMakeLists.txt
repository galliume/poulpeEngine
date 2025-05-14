target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES   
      FILES
        WinServer.cppm
        WinSocket.cppm
        UnixServer.cppm
        UnixSocket.cppm
        Socket.cppm
        Server.cppm
  PRIVATE 
    WinServer.cpp
    WinSocket.cpp
    UnixServer.cpp
    UnixSocket.cpp
    Socket.cpp
    Server.cpp)