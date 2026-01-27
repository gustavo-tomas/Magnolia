#pragma once

#include "magnolia/core/types.hpp"

namespace mag
{
    namespace mem
    {
        // Copy n bytes from src to dst (safer wrapper for memcpy)
        void copy(void* dst, const u64 dst_size, const void* const src, const u64 src_size, const u64 data_size);
    };  // namespace mem
};  // namespace mag
