#pragma once

// See this: https://github.com/dwjclark11/Scion2D/blob/master/SCION_CORE/src/Scripting/GlmLuaBindings.cpp

#include "core/logger.hpp"
#include "core/types.hpp"
#include "sol/sol.hpp"

namespace mag
{
    using namespace math;

    class LuaBindings
    {
        public:
            inline static void create_math_bindings(sol::state& lua)
            {
                create_vec2_bind(lua);
                create_vec3_bind(lua);
                create_vec4_bind(lua);

                math_free_functions(lua);
                math_constants(lua);
            }

        private:
            inline static void create_vec2_bind(sol::state& lua)
            {
                auto vec2_multiply_overloads = sol::overload([](const vec2& v1, const vec2& v2) { return v1 * v2; },
                                                             [](const vec2& v1, f32 value) { return v1 * value; },
                                                             [](f32 value, const vec2& v1) { return v1 * value; });

                auto vec2_divide_overloads = sol::overload([](const vec2& v1, const vec2& v2) { return v1 / v2; },
                                                           [](const vec2& v1, f32 value) { return v1 / value; },
                                                           [](f32 value, const vec2& v1) { return v1 / value; });

                auto vec2_addition_overloads = sol::overload([](const vec2& v1, const vec2& v2) { return v1 + v2; },
                                                             [](const vec2& v1, f32 value) { return v1 + value; },
                                                             [](f32 value, const vec2& v1) { return v1 + value; });

                auto vec2_subtraction_overloads = sol::overload([](const vec2& v1, const vec2& v2) { return v1 - v2; },
                                                                [](const vec2& v1, f32 value) { return v1 - value; },
                                                                [](f32 value, const vec2& v1) { return v1 - value; });

                lua.new_usertype<vec2>(
                    "vec2", sol::call_constructor, sol::constructors<vec2(f32), vec2(f32, f32)>(), "x", &vec2::x, "y",
                    &vec2::y, sol::meta_function::multiplication, vec2_multiply_overloads, sol::meta_function::division,
                    vec2_divide_overloads, sol::meta_function::addition, vec2_addition_overloads,
                    sol::meta_function::subtraction, vec2_subtraction_overloads, "length",
                    [](const vec2& v) { return length(v); }, "lengthSq", [](const vec2& v) { return length2(v); },
                    "normalize", [](const vec2& v1) { return normalize(v1); }, "normalize2",
                    [](const vec2& v1, const vec2& v2) { return normalize(v2 - v1); }, "nearly_zero_x",
                    [](const vec2& v) { return epsilonEqual(v.x, 0.f, 0.001f); }, "nearly_zero_y",
                    [](const vec2& v) { return epsilonEqual(v.y, 0.f, 0.001f); });
            }

            inline static void create_vec3_bind(sol::state& lua)
            {
                auto vec3_multiply_overloads = sol::overload([](const vec3& v1, const vec3& v2) { return v1 * v2; },
                                                             [](const vec3& v1, f32 value) { return v1 * value; },
                                                             [](f32 value, const vec3& v1) { return v1 * value; });

                auto vec3_divide_overloads = sol::overload([](const vec3& v1, const vec3& v2) { return v1 / v2; },
                                                           [](const vec3& v1, f32 value) { return v1 / value; },
                                                           [](f32 value, const vec3& v1) { return v1 / value; });

                auto vec3_addition_overloads = sol::overload([](const vec3& v1, const vec3& v2) { return v1 + v2; },
                                                             [](const vec3& v1, f32 value) { return v1 + value; },
                                                             [](f32 value, const vec3& v1) { return v1 + value; });

                auto vec3_subtraction_overloads = sol::overload([](const vec3& v1, const vec3& v2) { return v1 - v2; },
                                                                [](const vec3& v1, f32 value) { return v1 - value; },
                                                                [](f32 value, const vec3& v1) { return v1 - value; });

                lua.new_usertype<vec3>(
                    "vec3", sol::call_constructor, sol::constructors<vec3(f32), vec3(f32, f32, f32)>(), "x", &vec3::x,
                    "y", &vec3::y, "z", &vec3::z, sol::meta_function::multiplication, vec3_multiply_overloads,
                    sol::meta_function::division, vec3_divide_overloads, sol::meta_function::addition,
                    vec3_addition_overloads, sol::meta_function::subtraction, vec3_subtraction_overloads, "length",
                    [](const vec3& v) { return length(v); }, "lengthSq", [](const vec3& v) { return length2(v); },
                    "normalize", [](const vec3& v1) { return normalize(v1); }, "normalize2",
                    [](const vec3& v1, const vec3& v2) { return normalize(v2 - v1); }, "nearly_zero_x",
                    [](const vec3& v) { return epsilonEqual(v.x, 0.f, 0.001f); }, "nearly_zero_y",
                    [](const vec3& v) { return epsilonEqual(v.y, 0.f, 0.001f); }, "nearly_zero_z",
                    [](const vec3& v) { return epsilonEqual(v.z, 0.f, 0.001f); });
            }

            inline static void create_vec4_bind(sol::state& lua)
            {
                auto vec4_multiply_overloads = sol::overload([](const vec4& v1, const vec4& v2) { return v1 * v2; },
                                                             [](const vec4& v1, f32 value) { return v1 * value; },
                                                             [](f32 value, const vec4& v1) { return v1 * value; });

                auto vec4_divide_overloads = sol::overload([](const vec4& v1, const vec4& v2) { return v1 / v2; },
                                                           [](const vec4& v1, f32 value) { return v1 / value; },
                                                           [](f32 value, const vec4& v1) { return v1 / value; });

                auto vec4_addition_overloads = sol::overload([](const vec4& v1, const vec4& v2) { return v1 + v2; },
                                                             [](const vec4& v1, f32 value) { return v1 + value; },
                                                             [](f32 value, const vec4& v1) { return v1 + value; });

                auto vec4_subtraction_overloads = sol::overload([](const vec4& v1, const vec4& v2) { return v1 - v2; },
                                                                [](const vec4& v1, f32 value) { return v1 - value; },
                                                                [](f32 value, const vec4& v1) { return v1 - value; });

                lua.new_usertype<vec4>(
                    "vec4", sol::call_constructor, sol::constructors<vec4(f32), vec4(f32, f32, f32, f32)>(), "x",
                    &vec4::x, "y", &vec4::y, "z", &vec4::z, "w", &vec4::w, sol::meta_function::multiplication,
                    vec4_multiply_overloads, sol::meta_function::division, vec4_divide_overloads,
                    sol::meta_function::addition, vec4_addition_overloads, sol::meta_function::subtraction,
                    vec4_subtraction_overloads, "length", [](const vec4& v) { return length(v); }, "lengthSq",
                    [](const vec4& v) { return length2(v); }, "normalize", [](const vec4& v1) { return normalize(v1); },
                    "normalize2", [](const vec4& v1, const vec4& v2) { return normalize(v2 - v1); }, "nearly_zero_x",
                    [](const vec4& v) { return epsilonEqual(v.x, 0.f, 0.001f); }, "nearly_zero_y",
                    [](const vec4& v) { return epsilonEqual(v.y, 0.f, 0.001f); }, "nearly_zero_z",
                    [](const vec4& v) { return epsilonEqual(v.z, 0.f, 0.001f); }, "nearly_zero_w",
                    [](const vec4& v) { return epsilonEqual(v.w, 0.f, 0.001f); });
            }

            /*
             * Some helper math functions
             */
            inline static void math_free_functions(sol::state& lua)
            {
                lua.set_function("distance", sol::overload([](vec2& a, vec2& b) { return distance(a, b); },
                                                           [](vec3& a, vec3& b) { return distance(a, b); },
                                                           [](vec4& a, vec4& b) { return distance(a, b); }));

                lua.set_function("lerp", [](f32 a, f32 b, f32 t) { return std::lerp(a, b, t); });
                lua.set_function(
                    "clamp", sol::overload([](f32 value, f32 min, f32 max) { return std::clamp(value, min, max); },
                                           [](f64 value, f64 min, f64 max) { return std::clamp(value, min, max); },
                                           [](i32 value, i32 min, i32 max) { return std::clamp(value, min, max); }));

                lua.set_function("distance", sol::overload([](vec2& a, vec2& b) { return distance(a, b); },
                                                           [](vec3& a, vec3& b) { return distance(a, b); },
                                                           [](vec4& a, vec4& b) { return distance(a, b); }));

                lua.set_function(
                    "nearly_zero",
                    sol::overload([](const vec2& v)
                                  { return epsilonEqual(v.x, 0.f, 0.001f) && epsilonEqual(v.y, 0.f, 0.001f); },
                                  [](const vec3& v) {
                                      return epsilonEqual(v.x, 0.f, 0.001f) && epsilonEqual(v.y, 0.f, 0.001f) &&
                                             epsilonEqual(v.z, 0.f, 0.001f);
                                  }));
            }

            inline static void math_constants(sol::state& lua)
            {
                lua.set("PI", 3.14159265359f);
                lua.set("TWO_PI", 6.28318530717f);
                lua.set("PI_SQUARED", 9.86960440108f);
                lua.set("PI_OVER_2", 1.57079632679f);
                lua.set("PI_OVER_4", 0.78539816339f);
                lua.set("PHI", 1.6180339887498948482045868343656381f);
                lua.set("EULER", 2.71828182845904523536f);

                lua.set("SQRT_2", 1.4142135623730950488016887242097f);
                lua.set("SQRT_3", 1.7320508075688772935274463415059f);
                lua.set("INV_SQRT_2", 0.70710678118654752440084436210485f);
                lua.set("INV_SQRT_3", 0.57735026918962576450914878050196f);
            }
    };
};  // namespace mag
