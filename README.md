# Poulpe Engine
## Game Engine with a vulkan renderer

| **CI Service** | Build Status |
|:---------------|-------------:|
| GitHub Actions | [![CMake](https://github.com/galliume/poulpeEngine/actions/workflows/cmake.yml/badge.svg?branch=main)](https://github.com/galliume/poulpeEngine/actions/workflows/cmake.yml)


### This project is a never ending educationnal exercice about game engine and vulkan.

[State of the project](https://github.com/users/galliume/projects/4/views/1)

[In video](https://www.youtube.com/watch?v=C7p9z6LhAig&list=PL4-Os8BWDCPmZt5HvJrSo6QDHkD9J4fJF)

<img src="https://github.com/galliume/poulpeEngine/blob/main/screenshots/engine.jpg" width="800" height="600">
<img src="https://github.com/galliume/poulpeEngine/blob/main/screenshots/anim.gif" width="800" height="450">
<img src="https://github.com/galliume/poulpeEngine/blob/main/screenshots/rungholt.png" width="800" height="450">

[PoulpeEdit](https://github.com/galliume/poulpeEdit) will be the GUI editor that will use PoulpeEngine to edit a level, coming soon.

### To compile && execute 

Install the latest Vulkan SDK https://vulkan.lunarg.com/ and be sure to have the env variable VULKAN_SDK properly set to your Vulkan SDK installation folder.

Compile shaders (Windows && Linux) : 
```
bin/plpShadersCompilation.sh
```	

Building help:
```
bin/build.sh -h
```	

Generate the project with CMake and build it:
```
bin/build.sh -c -r -b
```

Or just configure it
```
bin/build.sh -c
``` 

Then open the .sln with VisualStudio

### All meshes can be found here:

Survival backpack: https://sketchfab.com/3d-models/survival-guitar-backpack-799f8c4511f84fab8c3f12887f7e6b36

Sponza: https://casual-effects.com/data/index.html

Dragon: https://casual-effects.com/data/index.html

Moon: https://free3d.com/3d-model/moon-photorealistic-2k-853071.html

Just download, unzip and copy paste in assets/mesh.

All levels are described in config/levels, for those meshes they should be fine as it is.

Models downloaded from Morgan McGuire's [Computer Graphics Archive](https://casual-effects.com/data)

[help to configure the project](https://github.com/galliume/poulpeEngine/wiki/Env-config)

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
