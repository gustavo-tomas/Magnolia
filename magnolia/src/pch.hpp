#ifndef _PCH_HEADER_
#define _PCH_HEADER_

// Don't include this anywhere besides the corresponding .cpp file

// First party headers
#include "magnolia/core/assert.hpp"
#include "magnolia/core/keys.hpp"
#include "magnolia/core/logger.hpp"
#include "magnolia/core/types.hpp"
#include "magnolia/math/types.hpp"

// Third party headers
#include <vulkan/vulkan.h>

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
