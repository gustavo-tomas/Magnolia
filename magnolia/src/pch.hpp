#ifndef _PCH_HEADER_
#define _PCH_HEADER_

// Don't include this anywhere besides the corresponding .cpp file

// First party headers
#include "core/assert.hpp"
#include "core/keys.hpp"
#include "core/logger.hpp"
#include "core/math.hpp"
#include "core/types.hpp"

// Third party headers
#include <vulkan/vulkan.hpp>

#include "SDL.h"
#include "SDL_vulkan.h"

// STL headers
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <source_location>
#include <string>
#include <thread>
#include <typeindex>
#include <unordered_map>
#include <vector>

#endif  // _PCH_HEADER_
