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

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtx/hash.hpp>

#include "vulkan/vulkan.h"

#include "Rebulk/Core/Log.h"
