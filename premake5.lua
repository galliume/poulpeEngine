workspace "Rebulkan"
	architecture "x64"
	startproject "Rebulkan"
	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "vendor/GLFW/include"
IncludeDir["GLM"] = "vendor/GLM"
IncludeDir["vulkan"] = "vendor/vulkan/include"

include "vendor/GLFW"
include "vendor/vulkan"

project "Rebulkan"
	location ""
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir  ("bin/" .. outputdir  .. "/%{prj.name}")
	objdir  ("bin-int/" .. outputdir  .. "/%{prj.name}")

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
		"vendor/imgui/backends/imgui_impl_glfw.h",
		"vendor/imgui/backends/imgui_impl_opengl3.h",
		"vendor/imgui/backends/imgui_impl_glfw.cpp",
		"vendor/imgui/backends/imgui_impl_opengl3.cpp",
		"vendor/imgui/backends/imgui_impl_opengl3_loader.h"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs
	{
		"src",
		"vendor/spdlog/include",
		"vendor/imgui",
		"vendor/stb_image",
		"vendor/tiny_obj_loader",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLM}",
		"%{IncludeDir.vulkan}",
	}

	links 
	{		
		"GLFW",
		"vendor/vulkan/Lib/vulkan-1.lib"
	}
	
	filter "system:windows"
		systemversion "latest"

		defines
		{
			"GLFW_INCLUDE_NONE",
			"_CRT_SECURE_NO_WARNINGS"
		}

	filter "configurations:Debug"
		defines "RBL_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "RBL_RELEASE"
		runtime "Release"
		symbols "on"

	filter "configurations:Dist"
		defines "RBL_DIST"
		runtime "Release"
		symbols "on"

	filter { "system:windows", "configurations:Debug" }
        buildoptions { "/MTd" }

	filter { "system:windows", "configurations:Release" }
        buildoptions { "/MT" }

	filter { "system:windows", "configurations:Dist" }
        buildoptions {"/MT" }