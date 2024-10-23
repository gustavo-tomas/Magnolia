#pragma once

// See this: https://github.com/dwjclark11/Scion2D/blob/master/SCION_CORE/src/Scripting/GlmLuaBindings.cpp

#include "core/logger.hpp"
#include "core/math.hpp"
#include "core/types.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "sol/sol.hpp"
#pragma clang diagnostic pop

namespace mag
{
    using namespace math;

    class LuaBindings
    {
        public:
            static void create_lua_bindings(sol::state& lua);

        private:
            static void create_vec2_bind(sol::state& lua);
            static void create_vec3_bind(sol::state& lua);
            static void create_vec4_bind(sol::state& lua);

            static void math_free_functions(sol::state& lua);
            static void math_constants(sol::state& lua);
            static void input_bindings(sol::state& lua);
    };
};  // namespace mag
