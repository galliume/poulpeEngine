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

if os.host() == "windows" then
	include "vendor/GLFW"
	include "vendor/vulkan"
end

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
		"vendor/imgui/imgui.h",
		"vendor/imgui/imgui.cpp",
		"vendor/imgui/imgui_demo.cpp",
		"vendor/imgui/imgui_draw.cpp",
		"vendor/imgui/imgui_tables.cpp",
		"vendor/imgui/imgui_widgets.cpp",
		"vendor/imgui/backends/imgui_impl_glfw.h",
		"vendor/imgui/backends/imgui_impl_glfw.cpp",
		"vendor/imgui/backends/imgui_impl_vulkan.h",
		"vendor/imgui/backends/imgui_impl_vulkan.cpp",
		"vendor/tiny_obj_loader/tiny_obj_loader.h"
	}

	includedirs
	{
		"src",
		"vendor/spdlog/include",
		"vendor/imgui",
		"vendor/imgui/backends",
		"vendor/stb_image",
		"vendor/tiny_obj_loader",
		"%{IncludeDir.GLM}",
	}

	filter "system:windows"

		systemversion "latest"

		includedirs
		{
			"%{IncludeDir.GLFW}",
			"%{IncludeDir.GLM}",
			"%{IncludeDir.vulkan}",
		}

		links 
		{		
			"GLFW",
			"vendor/vulkan/Lib/vulkan-1.lib"
		}
		
		defines
		{
			"GLFW_INCLUDE_NONE",
			"WIN32_LEAN_AND_MEAN",
			"_NO_DEBUG_HEAP=1"
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

	filter { "system:windows", "configurations:Debug" }
        buildoptions { "/MTd" }

	filter { "system:windows", "configurations:Release" }
        buildoptions { "/MT" }

	filter { "system:windows", "configurations:Dist" }
        buildoptions {"/MT" }

	filter "system:linux"
		systemversion "latest"

		defines
		{
			"GLFW_INCLUDE_NONE",
		}
	
		links { "glfw", "vulkan", "GL", "GLU", "dl",  "pthread" }