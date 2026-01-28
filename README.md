# 🐙 Poulpe Engine: A Modern C++23 Vulkan Game Engine

A continuously evolving, cross-platform game engine built from the ground up using **C++23** and powered by the **Vulkan** graphics API. This project is a never-ending educational exercise, covering various advanced topics such as graphics pipeline development, modern C++ patterns, low-level mathematics, and robust build systems.

## ✨ Project Status & Highlights

| **CI Service** | Build Status |
|:---------------|-------------:|
| GitHub Actions | [![PoulpeEngine](https://github.com/galliume/poulpeEngine/actions/workflows/CI.yml/badge.svg?branch=main)](https://github.com/galliume/poulpeEngine/actions/workflows/CI.yml)

### Key Features

* **Modern C++23:** Leveraging the latest C++ standard for clean, efficient, and expressive code.
* **Vulkan Renderer:** Utilizing the high-performance, explicit graphics API for unparalleled control over the GPU.
* **Advanced Rendering:** Includes support for modern features like **glTF 2.0 asset loading** and an extensible rendering pipeline.
* **Unified Image Layouts:** Implements the `VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME` for streamlined texture and image management.
* **Cross-Platform Ready:** Designed to compile and run on Windows and Linux (with editor support currently focused on Windows).

### Media & Development

* **Development Blog:** Follow the journey and technical deep-dives on [Mr Poulpe blog](https://mrpoulpe.substack.com/).
* **Project Kanban:** See what's next and the current development state on the [Project Board](https://github.com/users/galliume/projects/4/views/1).
* **Video Showcase:** Watch the engine in action on [YouTube](https://www.youtube.com/@galliume8395/videos).

### Screenshots

See the engine rendering advanced glTF assets:

| Basic Scene | PBR Materials Test | Rungholt |
| :---: | :---: | :---: |
| <img src="https://github.com/galliume/poulpeEngine/blob/main/screenshots/plp.png" width="400"> | <img src="https://github.com/galliume/poulpeEngine/blob/main/screenshots/helmet.png" width="400"> | <img src="https://github.com/galliume/poulpeEngine/blob/main/screenshots/rungholt.png" width="400"> |

---

## 🛠️ Compilation & Execution

### Prerequisites

To build and run the Poulpe Engine, you need recent and specific tooling:

1.  **C++ Compiler:** A recent version of **Clang (22.x)** with C++26 support.
2.  **Build System:** **CMake (4.x)**.
3.  **Vulkan SDK:** The latest official [Vulkan SDK](https://vulkan.lunarg.com/).
    * Ensure the environment variables **`VULKAN_SDK`**, **`Vulkan_LIBRARY`**, and **`Vulkan_INCLUDE_DIR`** are correctly configured.
4.  **GPU:** A recent GPU with support for the **`VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME`** extension, as this is currently used without fallback.

### Platform-Specific Setup

#### 💻 Windows
* **Toolchain:** Use **`llvm-mingw`** from [mstorsjo/llvm-mingw](https://github.com/mstorsjo/llvm-mingw/releases).
    * Add the `bin` folder to your `$PATH`.
    * Set the `CMAKE_SYSROOT` variable (or update `mingw-toolchain.cmake` directly).
* **Editor:** The engine editor relies on **Tcl/Tk 9.0**.
    * **Recommended:** Install via the [MagicSplat installer](https://www.magicsplat.com/tcl-installer/index.html).
    * Alternatively: [Download](https://www.tcl-lang.org/software/tcltk/9.0.html) the source and compile manually.

#### 🐧 Linux
* Tcl/Tk can be installed easily via your distribution's favorite package manager.

### Build Steps

1.  **Compile Shaders** (Windows & Linux):
    ```bash
    bin/shaders_compil.sh
    ```

2.  **List Presets:** Check available build configurations.
    ```bash
    cmake --list-presets
    # Available presets:
    # "windows-debug"   - Debug
    # "windows-release" - Release
    # "linux-debug"     - Debug
    # "linux-release"   - Release
    ```

3.  **Configure:** Use a preset to set up the build environment (Example: Windows Release).
    ```bash
    cmake --preset windows-release
    ```

4.  **Compile:** Build the project.
    ```bash
    cmake --build --preset windows-release
    ```

### Running the Engine

| Target | Command | Notes |
| :--- | :--- | :--- |
| **Engine** | `build/windows-release/PoulpeEngine.exe` | Run from the root directory. |
| **Editor** | `build/windows-release/PoulpeEngine.exe -E` | Windows only for the moment. |
| **IDE** | Open the project with a CMake-compatible IDE (VS 2022, VS Code, etc.). | |

---

## 🎨 Content & Configuration

### Level & Asset Setup

1.  **Assets:** Download glTF samples from [glTF-Sample-Assets](https://github.com/KhronosGroup/glTF-Sample-Assets) or meshes from Morgan McGuire's [Computer Graphics Archive](https://casual-effects.com/data).
2.  **Placement:** Unzip and copy/paste assets into the **`assets/mesh`** directory.
3.  **Level Configuration:** Levels are defined in **`config/levels`**.
4.  **Default Level:** To change the level loaded on startup, edit `config/poulpeEngine.json` and update the `defaultLevel` key with the target level filename.

> ℹ️ **Need Help?** A detailed environment configuration guide is available on the [Wiki](https://github.com/galliume/poulpeEngine/wiki/Env-config).

### 🕹️ Keyboard Shortcuts (AZERTY Layout)

| Action | Key |
| :--- | :--- |
| **Free / Lock Mouse** | `Left Ctrl` |
| **Forward** | `Z` |
| **Backward** | `S` (Changed from Q to S for standard WASD-to-AZERTY mapping) |
| **Strafe Left** | `Q` |
| **Strafe Right** | `D` |
| **Up** | `E` |
| **Down** | `A` |

---

## ✅ glTF 2.0 Feature Conformance

The engine is actively tested against the official glTF sample assets to ensure correct rendering and feature implementation.

| Feature Area | Supported | Feature Area | Supported | Feature Area | Supported |
| :--- | :---: | :--- | :---: | :--- | :---: |
| AlphaBlendModeTest | **[x]** | CompareIor | [ ] | MorphPrimitivesTest | [ ] |
| AnisotropyRotationTest | [ ] | CompareIridescence | [ ] | MorphStressTest | [ ] |
| AnisotropyStrengthTest | [ ] | CompareMetallic | [ ] | MultiUVTest | [ ] |
| ClearCoatTest | [ ] | CompareNormal | **[x]** | NegativeScaleTest | [ ] |
| CompareAlphaCoverage | [ ] | CompareRoughness | [ ] | NodePerformanceTest | [ ] |
| CompareAmbientOcclusion | [ ] | CompareSheen | [ ] | NormalTangentMirrorTest | **[x]** |
| CompareAnisotropy | [ ] | CompareSpecular | [ ] | NormalTangentTest | **[x]** |
| CompareBaseColor | **[x]** | CompareTransmission | [ ] | OrientationTest | **[x]** |
| CompareClearcoat | [ ] | CompareVolume | [ ] | PlaysetLightTest | [ ] |
| CompareDispersion | [ ] | DispersionTest | [ ] | PrimitiveModeNormalsTest | [ ] |
| CompareEmissiveStrength | [ ] | EmissiveStrengthTest | [ ] | SheenTestGrid | [ ] |
| EnvironmentTest | [ ] | InterpolationTest | [ ] | SpecularTest | [ ] |
| IORTestGrid | [ ] | TextureCoordinateTest | **[x]** | TextureEncodingTest | [ ] |
| TextureLinearInterpolationTest | [ ] | TextureSettingsTest | **[x]** | TextureTransformMultiTest | [ ] |
| TextureTransformTest | [ ] | TransmissionRoughnessTest | [ ] | TransmissionTest | [ ] |
| TransmissionThinwallTestGrid | [ ] | UnlitTest | [ ] |

---

## 📚 Further Learning & Resources

This project stands on the shoulders of giants. Here is a list of recommended resources that have informed the engine's development and will be invaluable for contributors:

* **Vulkan Official:**
    * [Vulkan API Specification](https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/index.html)
    * [LunarG Vulkan Documentation](https://vulkan.lunarg.com/doc/sdk/1.2.162.1/linux/tutorial/html/00-intro.html)
    * [Vulkan API Samples (LunarG)](https://github.com/LunarG/VulkanSamples/tree/master/API-Samples)
* **Tutorials & Best Practices:**
    * [Vulkan Tutorial (vulkan-tutorial.com)](https://vulkan-tutorial.com/)
    * [Efficient Vulkan Renderer (zeux.io)](https://zeux.io/2020/02/27/writing-an-efficient-vulkan-renderer/)
    * [Vulkan in 30 minutes (RenderDoc)](https://renderdoc.org/vulkan-in-30-minutes.html)
    * [Vulkan Dos and Don'ts (NVIDIA)](https://developer.nvidia.com/blog/vulkan-dos-donts/)
* **Books:**
    * [Vulkan Programming Guide](https://www.amazon.fr/Vulkan-Programming-Guide-Official-Learning/dp/0134464540/)
    * [Game Engine Architecture (Third Edition)](https://www.amazon.fr/Engine-Architecture-Third-Jason-Gregory/dp/1138035459/)
    * [Foundations of Game Engine Development (Vol 1 & 2)](https://foundationsofgameenginedev.com/)
