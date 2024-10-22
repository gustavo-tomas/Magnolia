#pragma once

#include <vector>

#include "core/types.hpp"

namespace mag
{
    struct Buffer
    {
            Buffer() = default;
            Buffer(const u64 size) : data(size){};

            template <typename T>
            T* cast()
            {
                return reinterpret_cast<T*>(data.data());
            }

            u64 get_size() const { return data.size(); };

            std::vector<u8> data;
    };
};  // namespace mag
