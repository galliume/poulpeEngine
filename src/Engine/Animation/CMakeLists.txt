target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
      FILES
        AnimationTypes.cppm
        AnimationScript.cppm
        BoneAnimationScript.cppm
  PRIVATE
    AnimationScript.cpp
    BoneAnimationScript.cpp)