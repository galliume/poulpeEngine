add_library(EngineCommon INTERFACE)

target_link_libraries(
  EngineCommon

  INTERFACE
    fmt::fmt
    glm
    volk)