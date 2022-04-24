workspace "Rebulkan"
    architecture "x64"
    startproject "Rebulkan"
	
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
		
include "vendor/glfw"
include "vendor/imgui"
	
project "Rebulkan"
    location ""
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir  ("bin/" .. outputdir  .. "/%{prj.name}")
    objdir  ("bin-int/" .. outputdir  .. "/%{prj.name}")

    pchheader "rebulkpch.h"
    pchsource "src/rebulkpch.cpp"

    flags
    {
        "MultiProcessorCompile"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "IMGUI_IMPL_VULKAN_NO_PROTOTYPES"
    }

    files 
    {
        "src/**.h",
        "src/**.cpp",
        "vendor/stb_image/**.h",
        "vendor/stb_image/**.cpp",
        "vendor/GLM/glm/**.hpp",
        "vendor/GLM/glm/**.inl",
	    "vendor/imgui/imgui_build.cpp",
		"vendor/stb_image/stb_image.cpp",
        "vendor/tiny_obj_loader/tiny_obj_loader.h",
        "vendor/tiny_obj_loader/mapbox/earcut.hpp",
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

	filter { "system:windows", "configurations:Debug" }
        buildoptions { "/MDd" }

	filter { "system:windows", "configurations:Release" }
        buildoptions { "/MT" }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        symbols "on"

    filter "system:linux"
        systemversion "latest"
    
        links { "glfw", "vulkan", "GL", "GLU", "dl",  "pthread" }