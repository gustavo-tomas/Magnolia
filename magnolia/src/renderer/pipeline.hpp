#pragma once

#include "core/types.hpp"

namespace mag
{
    class Shader;

    class Pipeline
    {
        public:
            Pipeline(const Shader& shader);
            ~Pipeline();

            void bind();

            const void* get_layout() const;

        private:
            struct IMPL;
            unique<IMPL> impl;
    };
};  // namespace mag
