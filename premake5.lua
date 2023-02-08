workspace "Rebulkan"
    architecture "x64"
    startproject "Rebulkan"
    toolset "clang"

    configurations
    {
        "Debug",
        "Release"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "vendor/GLFW/include"
IncludeDir["GLM"] = "vendor/GLM"
IncludeDir["volk"] = "vendor/volk"
IncludeDir["imgui"] = "vendor/imgui"
IncludeDir["spdlog"] = "vendor/spdlog/include"
IncludeDir["tiny_obj_loader"] = "vendor/tiny_obj_loader"
IncludeDir["stb_image"] = "vendor/stb_image"
IncludeDir["miniaudio"] = "vendor/miniaudio"
IncludeDir["nlohmann"] = "vendor/nlohmann"

include "vendor/GLFW"
include "vendor/imgui"

project "Rebulkan"
    location ""
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"

    targetdir  ("bin/" .. outputdir  .. "/%{prj.name}")
    objdir  ("bin-int/" .. outputdir  .. "/%{prj.name}")

    pchheader "rebulkpch.hpp"
    pchsource "src/rebulkpch.cpp"

    flags
    {
        "MultiProcessorCompile"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
		"_CRT_APP",
        "IMGUI_IMPL_VULKAN_NO_PROTOTYPES"
    }

    files 
    {
        "src/**.hpp",
        "src/**.cpp",
        "vendor/stb_image/**.hpp",
        "vendor/stb_image/**.cpp",
        "vendor/GLM/glm/**.hpp",
        "vendor/GLM/glm/**.inl",
        "vendor/imgui/imgui_build.cpp",
        "vendor/stb_image/stb_image.cpp",
        "vendor/tiny_obj_loader/tiny_obj_loader.h",
        "vendor/tiny_obj_loader/mapbox/earcut.hpp",
        "vendor/miniaudio/miniaudio.cpp",
        "vendor/nlohmann/json.hpp",
        "shaders/*",
        "mesh/*"
    }

    includedirs
    {
        "src",
        IncludeDir,
        os.getenv("VULKAN_SDK").."/Include"
    }

    filter "system:windows"

        systemversion "latest"

        links
        {
            "GLFW", "ImGui"
        }
        
        buildmessage 'Compiling shaders'
        prebuildcommands { "./scripts/WindowsShadersCompile.bat" }

        buildmessage 'Copying assets'
        postbuildcommands 
        {
            '{COPY} "./imgui.ini" "%{cfg.targetdir}/imgui.ini"',
            '{COPY} "./assets" "%{cfg.targetdir}/assets"',
            '{COPY} "./config" "%{cfg.targetdir}/config"'
        }

        buildoptions {
            "-Wall",
            "-Wextra",
            "-Wfloat-equal",
            "-Wundef",
            "-Wcast-align",
            "-Wwrite-strings",
            "-Wlogical-op",
            "-Wmissing-declarations",
            "-Wredundant-decls",
            "-Wshadow",
            "-Woverloaded-virtual",
            "-Wno-c++98-compat",
            "-Wno-c++98-compat-pedantic"
        }
		
    defines { "_NO_DEBUG_HEAP=1" }

    filter { "system:windows", "configurations:Debug" }
        buildoptions { "/MDd" }
    
	filter { "system:windows", "configurations:Release" }
        buildoptions { "/MD" }
   
    filter "system:linux"
        systemversion "latest"

        links { "GLFW", "ImGui", "X11", "dl", "pthread" }
 
        buildmessage 'Compiling shaders'
        prebuildcommands { "./scripts/LinuxShadersCompile.sh" }

        buildmessage 'Copying assets'
        postbuildcommands 
        {
            '{COPY} "./imgui.ini" "%{cfg.targetdir}/imgui.ini"',
            '{COPY} "./assets" "%{cfg.targetdir}/assets"',
            '{COPY} "./config" "%{cfg.targetdir}/config"'
        }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"

        defines { "RBK_DEBUG" }

    filter "configurations:Release"
        runtime "Release"
        symbols "Off"
        optimize "On"
