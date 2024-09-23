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

        private:
            void* freetype = nullptr;
    };
};  // namespace mag
