# Rebulkan
## Game Engine with a vulkan renderer

### This project is a never ending educationnal exercice about game engine and vulkan.

[State of the project](https://github.com/galliume/rebulkan/projects/1)

[In video](https://www.youtube.com/watch?v=C7p9z6LhAig&list=PL4-Os8BWDCPmZt5HvJrSo6QDHkD9J4fJF)

![actual_state](screenshots/state.jpg?raw=true "actual status")
![actual_state2](screenshots/state_2.jpg?raw=true "actual status bis")
![actual_state3](screenshots/fog_2.gif?raw=true "actual status ter")

### To execute 

First compile shaders

Windows : .\scripts\WindowsShadersCompile.bat

Linux   : ./scripts/LinuxShadersCompile.sh (broken)

Then compile the project

Windows : WindowsGenerateProject.bat generate a visual studio code 2022 sln. 
Load in VS Studio and hit F5... (you can change the .bat file and use an older version of visual studio)

Linux   : LinuxGenerateProject.sh generate a make file. Then "make" it
```
clear && make clean && make && ./bin/your_archi/Rebulkan/Rebulkan
```

[help to configure the project](https://github.com/galliume/rebulkan/wiki/Env-config)

Thanks to [premake](https://premake.github.io/)

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
