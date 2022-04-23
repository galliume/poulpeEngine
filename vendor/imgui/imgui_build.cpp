#include "slnpch.h"

// Order matters here in order to avoid redefinitions
#include <backends/imgui_impl_opengl3.cpp>
#include <backends/imgui_impl_vulkan.cpp>
#ifdef SLN_WINDOWS
#include <backends/imgui_impl_dx11.cpp>
#endif
#include <backends/imgui_impl_glfw.cpp>
