target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
      BASE_DIRS .
      FILES
        Application.cppm
  PRIVATE
    Application.cpp)