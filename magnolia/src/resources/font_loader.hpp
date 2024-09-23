#pragma once

#include "core/types.hpp"

namespace mag
{
    struct Font;

    class FontLoader
    {
        public:
            FontLoader();
            ~FontLoader();

            b8 load(const str& file_path, Font* font);
            b8 is_extension_supported(const str& extension_with_dot);

        private:
            void* freetype = nullptr;
    };
};  // namespace mag
