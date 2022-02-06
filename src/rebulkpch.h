#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint> 
#include <fstream>
#include <iostream>
#include <list>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <numeric>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtx/hash.hpp>

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/details/log_msg.h"

#include "stb_image.h"

#include "vulkan/vulkan.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include "Rebulk/Core/Tools.h"
#include "Rebulk/Core/Log.h"
