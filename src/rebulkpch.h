#define NOMINMAX

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
#include <string.h>
#include <unordered_map>
#include <numeric>
#include <map>
#include <filesystem>
#include <cstddef>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtx/hash.hpp>

#include "stb_image.h"

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/details/log_msg.h"

#include "Rebulk/Core/Log.h"
