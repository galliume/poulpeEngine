message(STATUS "Setting install.")

install(TARGETS ${PROJECT_NAME}
    EXPORT PoulpeEngineTargets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    FILE_SET cxx_headers DESTINATION include
    FILE_SET cxx_modules DESTINATION include)

##################### Install ###########################

# install(TARGETS
#   ${PROJECT_NAME}
# RUNTIME
#   COMPONENT Runtime
# LIBRARY
#   COMPONENT Runtime
#   NAMELINK_COMPONENT Development
# ARCHIVE
#   COMPONENT Development
#   DESTINATION lib/static
# FILE_SET HEADERS
#   COMPONENT Development
# )

message(STATUS "install OK.")