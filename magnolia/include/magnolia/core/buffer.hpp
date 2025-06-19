#pragma once

#include <vector>

#include "magnolia/core/types.hpp"

namespace mag
{
    struct Buffer
    {
            Buffer();
            Buffer(const u64 size);

            template <typename T>
            T* cast()
            {
                return reinterpret_cast<T*>(data.data());
            }

            u64 get_size() const;

            std::vector<u8> data;
    };
};  // namespace mag
