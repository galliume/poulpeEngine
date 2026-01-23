add_library(Animation STATIC)

target_sources(Animation
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
      FILES
        AnimationScript.cppm
        BoneAnimationScript.cppm
  PRIVATE
    AnimationScript.cpp
    BoneAnimationScript.cpp)

target_link_libraries(
  Animation

  PRIVATE
    Components
    Core
    EngineCommon

  PUBLIC
    lua)

set_poulpe_definitions(Animation)
set_poulpe_links(Animation)
set_poulpe_compiler_flags(Animation)