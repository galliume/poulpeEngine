#define NOMINMAX

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint> 
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <numeric>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtx/hash.hpp>

#include "stb_image.h"
#include "miniaudio.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/details/log_msg.h"

#include "Rebulk/Core/Log.h"