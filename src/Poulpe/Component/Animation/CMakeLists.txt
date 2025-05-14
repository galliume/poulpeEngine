target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
      FILES
        AnimationComponent.cppm
        AnimationScript.cppm
        BoneAnimationComponent.cppm
        BoneAnimationScript.cppm
        MeshComponent.cppm
  PRIVATE
    AnimationScript.cpp
    BoneAnimationScript.cpp)