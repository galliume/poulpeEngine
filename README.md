# Poulpe Engine
## Game Engine with a vulkan renderer

| **CI Service** | Build Status |
|:---------------|-------------:|
| GitHub Actions | [![PoulpeEngine](https://github.com/galliume/poulpeEngine/actions/workflows/CI.yml/badge.svg?branch=main)](https://github.com/galliume/poulpeEngine/actions/workflows/CI.yml)

### This project is a never ending educationnal exercice about game engine and vulkan.

[Mr Poulpe blog](https://mrpoulpe.substack.com/)

[State of the project](https://github.com/users/galliume/projects/4/views/1)

[In video](https://www.youtube.com/@galliume8395/videos)

<img src="https://github.com/galliume/poulpeEngine/blob/main/screenshots/sponza.png" width="800" height="600">
<img src="https://github.com/galliume/poulpeEngine/blob/main/screenshots/helmet.png" width="800" height="450">

### To compile && execute 

You will need a recent version of Clang (20.x) and CMake (3.31 or 4.x), c++23 is needed.

For Windows you will need mingw64 https://www.mingw-w64.org/ and add the bin folder into your PATH. (you do not need any MSVC stuff.)

Install the latest Vulkan SDK https://vulkan.lunarg.com/ and be sure to have the env variable VULKAN_SDK properly set to your Vulkan SDK installation folder.

The editor (Windows only for the moment) is using TCL/TK 9.0, the easiest installation for Windows is to install via the 
[MagicSplat installer](https://www.magicsplat.com/tcl-installer/index.html)

Or [download](https://www.tcl-lang.org/software/tcltk/9.0.html) the source and compile it yourself.

On Linux you can install it easily via your favorite package manager.

Compile shaders (Windows && Linux) : 
```
bin/shaders_compil.sh
```	

List existing presets
```
cmake --list-presets

"windows-debug"              - Debug
"windows-release"            - Release
"linux-debug"                - Debug
"linux-release"              - Release
```

Configure with a preset
```
cmake --preset windows-release
``` 

Compile with a preset
```
cmake --build --preset windows-dereleasebug
```

Launch from the root directory
```
build/windows-release/PoulpeEngine.exe
```

or to run the editor (Windows only)

```
build/windows-release/PoulpeEngine.exe -E
```

Or open the project with a cmake able IDE (VS 2022, VS Code...)

### All meshes can be found here:

glTF samples can be fetch here: [glTF-Sample-Assets](https://github.com/KhronosGroup/glTF-Sample-Assets)

or

from Morgan McGuire's [Computer Graphics Archive](https://casual-effects.com/data)

Just download, unzip and copy paste in assets/mesh.

All levels are described in config/levels, for those meshes they should be fine as it is.

To change the level update config/poulpeEngine.json key defaultLevel, copy/paste the file name of the level to be loaded.

[help to configure the project](https://github.com/galliume/poulpeEngine/wiki/Env-config)

### Keyboard shortcuts (azerty)

free / lock mouse : left ctrl

forward : Z 

backward : Q

left : Q

right : D

up : E

down : A

### glTF tests

- [x] AlphaBlendModeTest
- [ ] AnisotropyRotationTest
- [ ] AnisotropyStrengthTest
- [ ] ClearCoatTest
- [ ] CompareAlphaCoverage
- [ ] CompareAmbientOcclusion
- [ ] CompareAnisotropy
- [x] CompareBaseColor
- [ ] CompareClearcoat
- [ ] CompareDispersion
- [ ] CompareEmissiveStrength
- [ ] CompareIor
- [ ] CompareIridescence
- [ ] CompareMetallic
- [x] CompareNormal
- [ ] CompareRoughness
- [ ] CompareSheen
- [ ] CompareSpecular
- [ ] CompareTransmission
- [ ] CompareVolume
- [ ] DispersionTest
- [ ] EmissiveStrengthTest
- [ ] EnvironmentTest
- [ ] InterpolationTest
- [ ] IORTestGrid
- [ ] MorphPrimitivesTest
- [ ] MorphStressTest
- [ ] MultiUVTest
- [ ] NegativeScaleTest
- [ ] NodePerformanceTest
- [x] NormalTangentMirrorTest
- [x] NormalTangentTest
- [x] OrientationTest
- [ ] PlaysetLightTest
- [ ] PrimitiveModeNormalsTest
- [ ] SheenTestGrid
- [ ] SpecularTest
- [x] TextureCoordinateTest
- [ ] TextureEncodingTest
- [ ] TextureLinearInterpolationTest
- [x] TextureSettingsTest
- [ ] TextureTransformMultiTest
- [ ] TextureTransformTest
- [ ] TransmissionRoughnessTest
- [ ] TransmissionTest
- [ ] TransmissionThinwallTestGrid
- [ ] UnlitTest

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
