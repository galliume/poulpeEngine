#define NOMINMAX

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/details/log_msg.h>

#include <chrono>
#include <volk.h>

#include "Poulpe/Core/Log.hpp"
#include "Poulpe/Core/Locator.hpp"

#include "Poulpe/Utils/ScopedTimer.hpp"
