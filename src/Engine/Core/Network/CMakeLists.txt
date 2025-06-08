target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES   
      FILES
        Socket.cppm
        Server.cppm
  PRIVATE 
    Socket.cpp
    Server.cpp)