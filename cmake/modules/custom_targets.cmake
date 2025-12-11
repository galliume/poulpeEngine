message(STATUS "Setting compiler targets.")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:${PROJECT_NAME}> $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
  COMMAND_EXPAND_LISTS)

add_custom_target(
  CompilShaders ${CMAKE_CURRENT_SOURCE_DIR}/bin/shadersCompilation.sh
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

message(STATUS "compiler targets OK.")