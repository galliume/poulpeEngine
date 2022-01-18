project "vulkan"
    kind "StaticLib"
    language "C++"
    
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
        "Include/vulkan/*.h",
		"Include/vulkan/*.hpp",
    }

    includedirs
	{
        "Include"
    }

	filter { "system:windows" }    
		buildoptions { "-std=c14", "-lgdi32" }
		systemversion "latest"
		staticruntime "on"

		defines 
		{ 
			"_GLFW_WIN32",
			"_CRT_SECURE_NO_WARNINGS"
		}
		
		buildoptions "/MT"

	filter { "system:linux" }    
		buildoptions { "-std=c17" }
		systemversion "latest"
		staticruntime "on"

		links { "dl", "pthread" }