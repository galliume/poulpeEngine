project "GLFW"
    kind "StaticLib"
    language "C"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin/int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "include/GLFW/glfw3.h",
        "include/GLFW/glfw3native.h",
        "src/context.c",
        "src/init.c",
        "src/input.c",
        "src/monitor.c",
        "src/vulkan.c",
        "src/window.c",
        "src/internal.h",
        "src/mappings.h",
    }

    filter "system:windows"
        systemversion "latest"

        files
        {
            "src/win32_init.c",
            "src/win32_joystick.c",
            "src/win32_time.c",
            "src/win32_thread.c",
            "src/win32_window.c",
            "src/win32_platform.h",
            "src/win32_monitor.c",
            "src/wgl_context.c",
            "src/wgl_context.h",
            "src/egl_context.c",
            "src/egl_context.h",
            "src/osmesa_context.c",
            "src/osmesa_context.h"
        }

        defines 
        { 
            "_GLFW_WIN32",
        }

    filter "system:linux"
        systemversion "latest"
        
        files
        {
            "src/posix_thread.c",
            "src/posix_time.c",
            "src/linux_joystick.c",
            "src/egl_context.c",
            "src/egl_context.h",
            "src/osmesa_context.c",
            "src/osmesa_context.h",
            "src/glx_context.h",
            "src/glx_context.c",
            "src/x11_init.c",
            "src/x11_platform.h",
            "src/x11_monitor.c",
            "src/x11_window.c",
            "src/xkb_unicode.h",
            "src/xkb_unicode.c"
        }
        
        defines 
        { 
            "_GLFW_X11",
        }

        links { "x11", "dl", "GL", "pthread" }
    
    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"