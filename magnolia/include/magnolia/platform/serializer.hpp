#pragma once

#include <sstream>

#include "magnolia/core/buffer.hpp"
#include "magnolia/core/types.hpp"
#include "magnolia/math/types.hpp"

// This systems uses ADL (https://en.cppreference.com/w/cpp/language/adl.html) to find user implementations. Just
// include this file after declaring functions and the ADL will search for functions in the same namespace.

namespace mag::fs
{
    template <typename T>
    concept basic_type = std::integral<T> || std::floating_point<T>;

    // Basic types

    template <basic_type T>
    inline void to_binary(std::ostringstream& ss, const T& data)
    {
        ss.write(reinterpret_cast<const c8*>(&data), sizeof(data));
    }

    template <basic_type T>
    inline void from_binary(std::istringstream& ss, T& data)
    {
        ss.read(reinterpret_cast<c8*>(&data), sizeof(data));
    }

    // Maths

    template <typename T>
    inline void to_binary(std::ostringstream& ss, const math::vector2<T>& data)
    {
        for (u32 i = 0; i < 2; i++)
        {
            to_binary(ss, data[i]);
        }
    }

    template <typename T>
    inline void to_binary(std::ostringstream& ss, const math::vector3<T>& data)
    {
        for (u32 i = 0; i < 3; i++)
        {
            to_binary(ss, data[i]);
        }
    }

    template <typename T>
    inline void to_binary(std::ostringstream& ss, const math::vector4<T>& data)
    {
        for (u32 i = 0; i < 4; i++)
        {
            to_binary(ss, data[i]);
        }
    }

    template <typename T>
    inline void from_binary(std::istringstream& ss, mag::math::vector2<T>& data)
    {
        for (u32 i = 0; i < 2; i++)
        {
            from_binary(ss, data[i]);
        }
    }

    template <typename T>
    inline void from_binary(std::istringstream& ss, mag::math::vector3<T>& data)
    {
        for (u32 i = 0; i < 3; i++)
        {
            from_binary(ss, data[i]);
        }
    }

    template <typename T>
    inline void from_binary(std::istringstream& ss, mag::math::vector4<T>& data)
    {
        for (u32 i = 0; i < 4; i++)
        {
            from_binary(ss, data[i]);
        }
    }

    // Vector

    template <typename T>
    inline void to_binary(std::ostringstream& ss, const std::vector<T>& data)
    {
        const u64 count = data.size();
        to_binary(ss, count);

        for (const T& element : data)
        {
            to_binary(ss, element);
        }
    }

    template <typename T>
    inline void from_binary(std::istringstream& ss, std::vector<T>& data)
    {
        u64 count = 0;
        ss.read(reinterpret_cast<c8*>(&count), sizeof(count));
        data.resize(count);

        for (T& element : data)
        {
            from_binary(ss, element);
        }
    }

    // Serialization

    template <typename T>
    inline void serialize(Buffer& buffer, const T& data)
    {
        std::ostringstream ss(std::ios::app | std::ios::binary);

        to_binary(ss, data);

        const str& result = ss.str();

        buffer.data = std::vector<u8>(result.begin(), result.end());
    }

    template <typename T>
    inline void deserialize(Buffer& buffer, T& data)
    {
        const str data_str(buffer.data.begin(), buffer.data.end());

        std::istringstream ss(data_str, std::ios::binary);

        from_binary(ss, data);
    }
};  // namespace mag::fs
