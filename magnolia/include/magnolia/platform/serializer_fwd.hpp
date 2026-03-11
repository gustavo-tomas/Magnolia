#pragma once

#include <sstream>

#include "magnolia/core/types.hpp"
#include "magnolia/math/types.hpp"

// Include this file first to make basic conversion functions visible. After defining your conversion functions to any
// other data type, include serializer.hpp at the end of the file.

namespace mag::fs
{
    // Basic types

    template <typename T>
    concept basic_type = std::integral<T> || std::floating_point<T>;

    template <basic_type T>
    inline void to_binary(std::ostringstream& ss, const T& data);

    template <basic_type T>
    inline void from_binary(std::istringstream& ss, T& data);

    // Maths

    template <i32 L, typename T, math::qualifier Q>
    inline void to_binary(std::ostringstream& ss, const math::vec<L, T, Q>& data);

    template <i32 L, typename T, mag::math::qualifier Q>
    inline void from_binary(std::istringstream& ss, mag::math::vec<L, T, Q>& data);

    // Vector

    template <typename T>
    inline void to_binary(std::ostringstream& ss, const std::vector<T>& data);

    template <typename T>
    inline void from_binary(std::istringstream& ss, std::vector<T>& data);
};  // namespace mag::fs
