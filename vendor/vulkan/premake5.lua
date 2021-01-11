project "vulkan"
    kind "StaticLib"
    language "C++"
    
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
        "Include/vulkan/*.h",
    }

    includedirs {
        "Include"
    }
    
    filter "system:windows"
        buildoptions { "-std=c11", "-lgdi32" }
        systemversion "latest"
        staticruntime "on"
        
        files
        {
            "Include/vulkan/vulkan.hpp"
        }

		defines 
		{ 
            "_GLFW_WIN32",
            "_CRT_SECURE_NO_WARNINGS"
        }

    filter { "system:windows", "configurations:Release" }
        buildoptions "/MT"