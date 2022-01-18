project "GLFW"
    kind "StaticLib"
    language "C"
    
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
        "include/GLFW/glfw3.h",
        "include/GLFW/glfw3native.h",
        "src/glfw_config.h",
    }
    
	filter "system:windows"
        buildoptions { "-std=c11", "-lgdi32" }
        systemversion "latest"
        staticruntime "on"
        
        files
        {
            "src/win32_init.c",
            "src/win32_joystick.c",
            "src/win32_monitor.c",
            "src/win32_time.c",
            "src/win32_thread.c",
            "src/win32_window.c"
        }

		defines 
		{ 
            "_GLFW_WIN32",
            "_CRT_SECURE_NO_WARNINGS"
        }


    filter "system:linux"
        buildoptions { "-std=c17" }
        systemversion "latest"
        staticruntime "on"
        
        files
        {
            "src/internal.h",
            "src/posix_thread.c",
            "src/posix_time.c",
            "src/linux_joystick.c",
            "src/wl_init.c",
            "src/wl_platform.h",
            "src/wl_monitor.c",
            "src/wl_window.c",
        }
        
        defines 
        { 
            "_GLFW_WAYLAND" 
        }

        links { "dl", "pthread" }
