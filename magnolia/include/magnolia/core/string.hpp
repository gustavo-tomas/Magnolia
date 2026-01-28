#pragma once

#include <vector>

#include "magnolia/core/types.hpp"

namespace mag
{
    namespace string
    {
        // Separate substrings by a delimiter
        void split(const str& s, const str& del, std::vector<str>& output);

        // Lower case (because c++ is icky)
        void to_lower(str& s);

        // Upper case (because c++ is icky)
        void to_upper(str& s);

        // Case insensitive string comparison
        b8 equalni(const str& a, const str& b, const u64 n);

        // Remove trailing spaces from the start and end
        void trim(str& s);

        // Remove trailing spaces from the start
        void triml(str& s);

        // Remove trailing spaces from the end
        void trimr(str& s);
    };  // namespace string
};  // namespace mag
