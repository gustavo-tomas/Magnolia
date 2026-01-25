#include "magnolia/core/memory.hpp"

#include <cstring>

#include "magnolia/core/assert.hpp"
#include "magnolia/core/types.hpp"

namespace mag
{
    namespace mem
    {
        // Copy n bytes from src to dst
        void copy(void* dst, const u64 dst_size, const void* const src, const u64 src_size, const u64 data_size)
        {
            MAG_ASSERT(src != nullptr, "Invalid src pointer");
            MAG_ASSERT(dst != nullptr, "Invalid dst pointer");

            MAG_ASSERT(data_size <= src_size, "Data size exceeds src size");
            MAG_ASSERT(data_size <= dst_size, "Data size exceeds dst size");

            std::memcpy(dst, src, data_size);
        }
    };  // namespace mem
};  // namespace mag
