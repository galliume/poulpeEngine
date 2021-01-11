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
	cppdialect "C++17"
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
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs
	{
		"src",
		"vendor/spdlog/include",
		"vendor/stb_image",
		"vendor/tiny_obj_loader",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLM}",
		"%{IncludeDir.vulkan}"
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
			"RBL_PLATFORM_WINDOWS",
			"RBL_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
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

	filter { "system:windows", "configurations:Release" }
        buildoptions "/MTd"