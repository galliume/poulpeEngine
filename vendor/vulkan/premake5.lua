project "vulkan"
    kind "StaticLib"
    language "C++"
    
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
        "Include/vulkan/*.h",
		"Include/vulkan/vulkan.hpp"
    }

    includedirs 
	{
        "Include"
    }
    
	buildoptions { "-std=c14", "-lgdi32" }
	systemversion "latest"
	staticruntime "on"

	filter { "system:windows" }    
		defines 
		{ 
			"_GLFW_WIN32",
			"_CRT_SECURE_NO_WARNINGS"
		}

    filter { "system:windows" }
		buildoptions "/MT"
       
    filter { "system:linux" }
        links { "dl", "pthread" }
