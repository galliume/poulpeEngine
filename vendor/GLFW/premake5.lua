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
            "src/win32_monitor.c",
            "src/win32_time.c",
            "src/win32_thread.c",
            "src/win32_window.c",
            "src/win32_platform.h",
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
            "_CRT_SECURE_NO_WARNINGS"
        }
        
    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"

    filter "system:linux"
        systemversion "latest"
        
        files
        {
            "src/posix_thread.c",
            "src/posix_time.c",
            "src/egl_context.c",
            "src/egl_context.h",
            "src/osmesa_context.c",
            "src/osmesa_context.h",
            "src/linux_joystick.c",
            "src/wl_init.c",
            "src/wl_platform.h",
            "src/wl_monitor.c",
            "src/wl_window.c",
            "src/wayland-xdg-shell-protocol.c",
            "src/wayland-xdg-decoration-protocol.c",
            "src/wayland-viewporter-protocol.c",
            "src/wayland-relative-pointer-unstable-v1-protocol.c",
            "src/wayland-pointer-constraints-unstable-v1-protocol.c",
            "src/wayland-idle-inhibit-unstable-v1-protocol.c",
            "src/xkb_unicode.h",
            "src/xkb_unicode.c"
        }
        
        defines 
        { 
            "GLFW_USE_WAYLAND",
            "_GLFW_WAYLAND",
        }

        links { "wayland-client", "dl", "GL", "pthread" }