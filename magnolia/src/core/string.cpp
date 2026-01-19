#include "magnolia/core/string.hpp"

#include <algorithm>

namespace mag
{
    namespace string
    {
        b8 equalni(const str& a, const str& b, const u64 n)
        {
            if (n > b.size() || n > a.size())
            {
                return false;
            }

            str str_a = a;
            str str_b = b;

            to_lower(str_a);
            to_lower(str_b);

            str_a = str_a.substr(0, n);
            str_b = str_b.substr(0, n);

            return str_a == str_b;
        }

        void triml(str& s)
        {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](const uc8 ch) { return !std::isspace(ch); }));
        }

        void trimr(str& s)
        {
            s.erase(std::find_if(s.rbegin(), s.rend(), [](const uc8 ch) { return !std::isspace(ch); }).base(), s.end());
        }

        void to_lower(str& s) { std::transform(s.begin(), s.end(), s.begin(), tolower); }

        void to_upper(str& s) { std::transform(s.begin(), s.end(), s.begin(), toupper); }
    };  // namespace string
};      // namespace mag
