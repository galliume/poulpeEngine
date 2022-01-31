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
        buildoptions { "-std=c14", "-lgdi32" }
        systemversion "latest"
        staticruntime "on"
        
        files
        {
            "src/context.c",
            "src/init.c",
            "src/input.c",
            "src/monitor.c",
            "src/vulkan.c",
            "src/window.c",
            "src/win32_init.c",
            "src/win32_joystick.c",
            "src/win32_monitor.c",
            "src/win32_time.c",
            "src/win32_thread.c",
            "src/win32_window.c",
            "src/wgl_context.c",
            "src/egl_context.c",
            "src/osmesa_context.c"
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
            "src/x11_init.c",
            "src/x11_platform.h",
            "src/x11_monitor.c",
            "src/x11_window.c",
        }
        
        defines 
        { 
            
        }

        links { "dl", "pthread" }
