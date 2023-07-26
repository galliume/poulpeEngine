# Rebulkan
## Game Engine with a vulkan renderer

| **CI Service** | Build Status |
|:---------------|-------------:|
| GitHub Actions | [![CMake](https://github.com/galliume/rebulkan/actions/workflows/cmake.yml/badge.svg?branch=main)](https://github.com/galliume/rebulkan/actions/workflows/cmake.yml)


### This project is a never ending educationnal exercice about game engine and vulkan.

[State of the project](https://github.com/users/galliume/projects/4/views/1)

[In video](https://www.youtube.com/watch?v=C7p9z6LhAig&list=PL4-Os8BWDCPmZt5HvJrSo6QDHkD9J4fJF)

![actual_state1](screenshots/viewport.jpg?raw=true "Rebulkan Engine GUI")
![actual_state2](screenshots/fog_2.gif?raw=true "Rebulkan 3D rendering")


### To execute 

Install the latest Vulkan SDK https://vulkan.lunarg.com/ and be sure to have the env variable VULKAN_SDK properly set to your Vulkan SDK installation folder.

Compile shaders (windows or linux) : 
```
./bin/rbkShadersCompilation.sh
```	

Generate the project and run with CMake :

```
clear && ./bin/rbkBuild.sh && ./build/Debug/Debug/Rebulkan
```

You can also generate and build yourself using CMake and the IDE you want.

By default the project is compiled with clang+ (see in ./bin/rbkBuild.sh)

rbkBuild.sh can take those options (here are the default values)

>-a: Build directory [default to CMAKE_BUILD_DIR="build"]
>
>-b: CXX compiler [default to CMAKE_CXX_COMPILER="clang++"]
>
>-c: C compiler [default to CMAKE_C_COMPILER="clang"]
>
>-d: Cmake build type [default to CMAKE_BUILD_TYPE="Debug"]
>
>-e: Clean build dir [default to REFRESH_BUILD_DIR=false]
>
>-f: Number of parallels jobs [default to CMAKE_J=8]
>
>-g: CMake toolset (-T) [default to CMAKE_TOOLSET="ClangCL"]
>
>-h Display help	
>
>-i: Build
>
>-j: Run Rebulkan
>
>-k: Build and Run
>
>-l: Use Ninja [default to USE_NINJA=true]

For exemple, if you want to build a Release in a folder named foo and force a clean refresh of the folder:

```
./bin/rbkBuild.sh -afoo -dRelease -etrue
```

[help to configure the project](https://github.com/galliume/rebulkan/wiki/Env-config)

### Keyboard shortcuts (azerty)

free / lock mouse : left ctrl

forward : Z 

backward : Q

left : Q

right : D

up : E

down : A

### Resources

[Vulkan Programming Guide](https://www.amazon.fr/Vulkan-Programming-Guide-Official-Learning/dp/0134464540/)

[Game Engine Architecture](https://www.amazon.fr/Engine-Architecture-Third-Jason-Gregory/dp/1138035459/)

[Vulkan API](https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/index.html)

[Vulkan API Samples](https://github.com/LunarG/VulkanSamples/tree/master/API-Samples)

[LunarG Vulkan](https://vulkan.lunarg.com/doc/sdk/1.2.162.1/linux/tutorial/html/00-intro.html)

[Foundations of Game Engine Development (vol 1&2)](https://foundationsofgameenginedev.com/)

[Vulkan Tutorial](https://vulkan-tutorial.com/)

[Efficient Vulkan Renderer](https://zeux.io/2020/02/27/writing-an-efficient-vulkan-renderer/)

[Vulkan in 30 minutes](https://renderdoc.org/vulkan-in-30-minutes.html)

[Do's and Don't](https://developer.nvidia.com/blog/vulkan-dos-donts/)
