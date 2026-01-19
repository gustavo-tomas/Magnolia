#pragma once

#include "magnolia/core/types.hpp"

namespace mag
{
    namespace string
    {
        // Lower case (because c++ is icky)
        void to_lower(str& s);

        // Upper case (because c++ is icky)
        void to_upper(str& s);

        // Case insensitive string comparison
        b8 equalni(const str& a, const str& b, const u64 n);

        // Remove trailing spaces from the start
        void triml(str& s);

        // Remove trailing spaces from the end
        void trimr(str& s);
    };  // namespace string
};      // namespace mag
