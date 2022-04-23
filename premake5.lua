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

if os.host() == "windows" then
    include "vendor/GLFW"
end

project "Rebulkan"
    location ""
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir  ("bin/" .. outputdir  .. "/%{prj.name}")
    objdir  ("bin-int/" .. outputdir  .. "/%{prj.name}")

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
        "vendor/imgui/imgui.h",
        "vendor/imgui/imgui.cpp",
        "vendor/imgui/imgui_draw.cpp",
        "vendor/imgui/imgui_tables.cpp",
        "vendor/imgui/imgui_widgets.cpp",
        "vendor/imgui/imgui_demo.cpp",
        "vendor/imgui/backends/imgui_impl_glfw.h",
        "vendor/imgui/backends/imgui_impl_glfw.cpp",
        "vendor/imgui/backends/imgui_impl_vulkan.h",
        "vendor/imgui/backends/imgui_impl_vulkan.cpp",
        "vendor/tiny_obj_loader/tiny_obj_loader.h",
        "vendor/tiny_obj_loader/mapbox/earcut.hpp",
        "shaders/*",
        "mesh/*"
    }

    includedirs
    {
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.volk}",
        "%{IncludeDir.GLM}",
        "vendor/spdlog/include",
        "vendor/imgui",
        "vendor/imgui/backends",
        "vendor/stb_image",
        "vendor/tiny_obj_loader",        
        os.getenv("VULKAN_SDK").."/Include",
        "src"
    }

    filter "system:windows"

        systemversion "latest"

        links 
        {       
            "GLFW"
        }
        

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        symbols "on"

    filter "configurations:Dist"
        runtime "Release"
        symbols "on"

    filter "system:linux"
        systemversion "latest"
    
        links { "glfw", "vulkan", "GL", "GLU", "dl",  "pthread" }