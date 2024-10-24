#include "scripting/lua_bindings.hpp"

#include "core/application.hpp"
#include "core/window.hpp"

namespace mag
{
    using namespace math;

    void LuaBindings::create_lua_bindings(sol::state& lua)
    {
        create_vec2_bind(lua);
        create_vec3_bind(lua);
        create_vec4_bind(lua);

        math_free_functions(lua);
        math_constants(lua);

        input_bindings(lua);
    }

    void LuaBindings::create_vec2_bind(sol::state& lua)
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
            "vec2", sol::call_constructor, sol::constructors<vec2(f32), vec2(f32, f32)>(), "x", &vec2::x, "y", &vec2::y,
            sol::meta_function::multiplication, vec2_multiply_overloads, sol::meta_function::division,
            vec2_divide_overloads, sol::meta_function::addition, vec2_addition_overloads,
            sol::meta_function::subtraction, vec2_subtraction_overloads, "length",
            [](const vec2& v) { return length(v); }, "lengthSq", [](const vec2& v) { return length2(v); }, "normalize",
            [](const vec2& v1) { return normalize(v1); }, "normalize2",
            [](const vec2& v1, const vec2& v2) { return normalize(v2 - v1); }, "nearly_zero_x",
            [](const vec2& v) { return epsilonEqual(v.x, 0.f, 0.001f); }, "nearly_zero_y",
            [](const vec2& v) { return epsilonEqual(v.y, 0.f, 0.001f); });
    }

    void LuaBindings::create_vec3_bind(sol::state& lua)
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
            "vec3", sol::call_constructor, sol::constructors<vec3(f32), vec3(f32, f32, f32)>(), "x", &vec3::x, "y",
            &vec3::y, "z", &vec3::z, sol::meta_function::multiplication, vec3_multiply_overloads,
            sol::meta_function::division, vec3_divide_overloads, sol::meta_function::addition, vec3_addition_overloads,
            sol::meta_function::subtraction, vec3_subtraction_overloads, "length",
            [](const vec3& v) { return length(v); }, "lengthSq", [](const vec3& v) { return length2(v); }, "normalize",
            [](const vec3& v1) { return normalize(v1); }, "normalize2",
            [](const vec3& v1, const vec3& v2) { return normalize(v2 - v1); }, "nearly_zero_x",
            [](const vec3& v) { return epsilonEqual(v.x, 0.f, 0.001f); }, "nearly_zero_y",
            [](const vec3& v) { return epsilonEqual(v.y, 0.f, 0.001f); }, "nearly_zero_z",
            [](const vec3& v) { return epsilonEqual(v.z, 0.f, 0.001f); });
    }

    void LuaBindings::create_vec4_bind(sol::state& lua)
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
            "vec4", sol::call_constructor, sol::constructors<vec4(f32), vec4(f32, f32, f32, f32)>(), "x", &vec4::x, "y",
            &vec4::y, "z", &vec4::z, "w", &vec4::w, sol::meta_function::multiplication, vec4_multiply_overloads,
            sol::meta_function::division, vec4_divide_overloads, sol::meta_function::addition, vec4_addition_overloads,
            sol::meta_function::subtraction, vec4_subtraction_overloads, "length",
            [](const vec4& v) { return length(v); }, "lengthSq", [](const vec4& v) { return length2(v); }, "normalize",
            [](const vec4& v1) { return normalize(v1); }, "normalize2",
            [](const vec4& v1, const vec4& v2) { return normalize(v2 - v1); }, "nearly_zero_x",
            [](const vec4& v) { return epsilonEqual(v.x, 0.f, 0.001f); }, "nearly_zero_y",
            [](const vec4& v) { return epsilonEqual(v.y, 0.f, 0.001f); }, "nearly_zero_z",
            [](const vec4& v) { return epsilonEqual(v.z, 0.f, 0.001f); }, "nearly_zero_w",
            [](const vec4& v) { return epsilonEqual(v.w, 0.f, 0.001f); });
    }

    /*
     * Some helper math functions
     */
    void LuaBindings::math_free_functions(sol::state& lua)
    {
        lua.set_function("distance", sol::overload([](vec2& a, vec2& b) { return distance(a, b); },
                                                   [](vec3& a, vec3& b) { return distance(a, b); },
                                                   [](vec4& a, vec4& b) { return distance(a, b); }));

        lua.set_function("lerp", [](f32 a, f32 b, f32 t) { return std::lerp(a, b, t); });
        lua.set_function("clamp",
                         sol::overload([](f32 value, f32 min, f32 max) { return std::clamp(value, min, max); },
                                       [](f64 value, f64 min, f64 max) { return std::clamp(value, min, max); },
                                       [](i32 value, i32 min, i32 max) { return std::clamp(value, min, max); }));

        lua.set_function("distance", sol::overload([](vec2& a, vec2& b) { return distance(a, b); },
                                                   [](vec3& a, vec3& b) { return distance(a, b); },
                                                   [](vec4& a, vec4& b) { return distance(a, b); }));

        lua.set_function("nearly_zero",
                         sol::overload([](const vec2& v)
                                       { return epsilonEqual(v.x, 0.f, 0.001f) && epsilonEqual(v.y, 0.f, 0.001f); },
                                       [](const vec3& v) {
                                           return epsilonEqual(v.x, 0.f, 0.001f) && epsilonEqual(v.y, 0.f, 0.001f) &&
                                                  epsilonEqual(v.z, 0.f, 0.001f);
                                       }));
    }

    void LuaBindings::math_constants(sol::state& lua)
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

    void LuaBindings::input_bindings(sol::state& lua)
    {
        // @NOTE: use pointers when registering app functions to prevent sol2 from freeing the memory when the
        // state is
        // deleted.
        lua.set_function("is_key_down", &Window::is_key_down, &get_application().get_window());
        lua.set_function("is_button_down", &Window::is_button_down, &get_application().get_window());

        lua.new_enum<Buttons>("Buttons", {{"Left", Buttons::Left},
                                          {"Middle", Buttons::Middle},
                                          {"Right", Buttons::Right},
                                          {"X1", Buttons::X1},
                                          {"X2", Buttons::X2}});

        lua.new_enum<Keys>("Keys", {
                                       {"Unknown", Keys::Unknown},
                                       {"Return", Keys::Return},
                                       {"Escape", Keys::Escape},
                                       {"Backspace", Keys::Backspace},
                                       {"Tab", Keys::Tab},
                                       {"Space", Keys::Space},
                                       {"Exclaim", Keys::Exclaim},
                                       {"Quotedbl", Keys::Quotedbl},
                                       {"Hash", Keys::Hash},
                                       {"Percent", Keys::Percent},
                                       {"Dollar", Keys::Dollar},
                                       {"Ampersand", Keys::Ampersand},
                                       {"Quote", Keys::Quote},
                                       {"Leftparen", Keys::Leftparen},
                                       {"Rightparen", Keys::Rightparen},
                                       {"Asterisk", Keys::Asterisk},
                                       {"Plus", Keys::Plus},
                                       {"Comma", Keys::Comma},
                                       {"Minus", Keys::Minus},
                                       {"Period", Keys::Period},
                                       {"Slash", Keys::Slash},
                                       {"_0", Keys::_0},
                                       {"_1", Keys::_1},
                                       {"_2", Keys::_2},
                                       {"_3", Keys::_3},
                                       {"_4", Keys::_4},
                                       {"_5", Keys::_5},
                                       {"_6", Keys::_6},
                                       {"_7", Keys::_7},
                                       {"_8", Keys::_8},
                                       {"_9", Keys::_9},
                                       {"Colon", Keys::Colon},
                                       {"Semicolon", Keys::Semicolon},
                                       {"Less", Keys::Less},
                                       {"Equals", Keys::Equals},
                                       {"Greater", Keys::Greater},
                                       {"Question", Keys::Question},
                                       {"At", Keys::At},

                                       /*
                                           Skip uppercase letters
                                       */

                                       {"Leftbracket", Keys::Leftbracket},
                                       {"Backslash", Keys::Backslash},
                                       {"Rightbracket", Keys::Rightbracket},
                                       {"Caret", Keys::Caret},
                                       {"Underscore", Keys::Underscore},
                                       {"Backquote", Keys::Backquote},
                                       {"a", Keys::a},
                                       {"b", Keys::b},
                                       {"c", Keys::c},
                                       {"d", Keys::d},
                                       {"e", Keys::e},
                                       {"f", Keys::f},
                                       {"g", Keys::g},
                                       {"h", Keys::h},
                                       {"i", Keys::i},
                                       {"j", Keys::j},
                                       {"k", Keys::k},
                                       {"l", Keys::l},
                                       {"m", Keys::m},
                                       {"n", Keys::n},
                                       {"o", Keys::o},
                                       {"p", Keys::p},
                                       {"q", Keys::q},
                                       {"r", Keys::r},
                                       {"s", Keys::s},
                                       {"t", Keys::t},
                                       {"u", Keys::u},
                                       {"v", Keys::v},
                                       {"w", Keys::w},
                                       {"x", Keys::x},
                                       {"y", Keys::y},
                                       {"z", Keys::z},

                                       {"Capslock", Keys::Capslock},

                                       {"F1", Keys::F1},
                                       {"F2", Keys::F2},
                                       {"F3", Keys::F3},
                                       {"F4", Keys::F4},
                                       {"F5", Keys::F5},
                                       {"F6", Keys::F6},
                                       {"F7", Keys::F7},
                                       {"F8", Keys::F8},
                                       {"F9", Keys::F9},
                                       {"F10", Keys::F10},
                                       {"F11", Keys::F11},
                                       {"F12", Keys::F12},

                                       {"Printscreen", Keys::Printscreen},
                                       {"Scrolllock", Keys::Scrolllock},
                                       {"Pause", Keys::Pause},
                                       {"Insert", Keys::Insert},
                                       {"Home", Keys::Home},
                                       {"Pageup", Keys::Pageup},
                                       {"Delete", Keys::Delete},
                                       {"End", Keys::End},
                                       {"Pagedown", Keys::Pagedown},
                                       {"Right", Keys::Right},
                                       {"Left", Keys::Left},
                                       {"Down", Keys::Down},
                                       {"Up", Keys::Up},

                                       {"Numlockclear", Keys::Numlockclear},
                                       {"Kp_divide", Keys::Kp_divide},
                                       {"Kp_multiply", Keys::Kp_multiply},
                                       {"Kp_minus", Keys::Kp_minus},
                                       {"Kp_plus", Keys::Kp_plus},
                                       {"Kp_enter", Keys::Kp_enter},
                                       {"Kp_1", Keys::Kp_1},
                                       {"Kp_2", Keys::Kp_2},
                                       {"Kp_3", Keys::Kp_3},
                                       {"Kp_4", Keys::Kp_4},
                                       {"Kp_5", Keys::Kp_5},
                                       {"Kp_6", Keys::Kp_6},
                                       {"Kp_7", Keys::Kp_7},
                                       {"Kp_8", Keys::Kp_8},
                                       {"Kp_9", Keys::Kp_9},
                                       {"Kp_0", Keys::Kp_0},
                                       {"Kp_period", Keys::Kp_period},

                                       {"Application", Keys::Application},
                                       {"Power", Keys::Power},
                                       {"Kp_equals", Keys::Kp_equals},
                                       {"F13", Keys::F13},
                                       {"F14", Keys::F14},
                                       {"F15", Keys::F15},
                                       {"F16", Keys::F16},
                                       {"F17", Keys::F17},
                                       {"F18", Keys::F18},
                                       {"F19", Keys::F19},
                                       {"F20", Keys::F20},
                                       {"F21", Keys::F21},
                                       {"F22", Keys::F22},
                                       {"F23", Keys::F23},
                                       {"F24", Keys::F24},
                                       {"Execute", Keys::Execute},
                                       {"Help", Keys::Help},
                                       {"Menu", Keys::Menu},
                                       {"Select", Keys::Select},
                                       {"Stop", Keys::Stop},
                                       {"Again", Keys::Again},
                                       {"Undo", Keys::Undo},
                                       {"Cut", Keys::Cut},
                                       {"Copy", Keys::Copy},
                                       {"Paste", Keys::Paste},
                                       {"Find", Keys::Find},
                                       {"Mute", Keys::Mute},
                                       {"Volumeup", Keys::Volumeup},
                                       {"Volumedown", Keys::Volumedown},
                                       {"Kp_comma", Keys::Kp_comma},
                                       {"Kp_equalsas400", Keys::Kp_equalsas400},

                                       {"Alterase", Keys::Alterase},
                                       {"Sysreq", Keys::Sysreq},
                                       {"Cancel", Keys::Cancel},
                                       {"Clear", Keys::Clear},
                                       {"Prior", Keys::Prior},
                                       {"Return2", Keys::Return2},
                                       {"Separator", Keys::Separator},
                                       {"Out", Keys::Out},
                                       {"Oper", Keys::Oper},
                                       {"Clearagain", Keys::Clearagain},
                                       {"Crsel", Keys::Crsel},
                                       {"Exsel", Keys::Exsel},

                                       {"Kp_00", Keys::Kp_00},
                                       {"Kp_000", Keys::Kp_000},
                                       {"Thousandsseparator", Keys::Thousandsseparator},
                                       {"Decimalseparator", Keys::Decimalseparator},
                                       {"Currencyunit", Keys::Currencyunit},
                                       {"Currencysubunit", Keys::Currencysubunit},
                                       {"Kp_leftparen", Keys::Kp_leftparen},
                                       {"Kp_rightparen", Keys::Kp_rightparen},
                                       {"Kp_leftbrace", Keys::Kp_leftbrace},
                                       {"Kp_rightbrace", Keys::Kp_rightbrace},
                                       {"Kp_tab", Keys::Kp_tab},
                                       {"Kp_backspace", Keys::Kp_backspace},
                                       {"Kp_a", Keys::Kp_a},
                                       {"Kp_b", Keys::Kp_b},
                                       {"Kp_c", Keys::Kp_c},
                                       {"Kp_d", Keys::Kp_d},
                                       {"Kp_e", Keys::Kp_e},
                                       {"Kp_f", Keys::Kp_f},
                                       {"Kp_xor", Keys::Kp_xor},
                                       {"Kp_power", Keys::Kp_power},
                                       {"Kp_percent", Keys::Kp_percent},
                                       {"Kp_less", Keys::Kp_less},
                                       {"Kp_greater", Keys::Kp_greater},
                                       {"Kp_ampersand", Keys::Kp_ampersand},
                                       {"Kp_dblampersand", Keys::Kp_dblampersand},
                                       {"Kp_verticalbar", Keys::Kp_verticalbar},
                                       {"Kp_dblverticalbar", Keys::Kp_dblverticalbar},
                                       {"Kp_colon", Keys::Kp_colon},
                                       {"Kp_hash", Keys::Kp_hash},
                                       {"Kp_space", Keys::Kp_space},
                                       {"Kp_at", Keys::Kp_at},
                                       {"Kp_exclam", Keys::Kp_exclam},
                                       {"Kp_memstore", Keys::Kp_memstore},
                                       {"Kp_memrecall", Keys::Kp_memrecall},
                                       {"Kp_memclear", Keys::Kp_memclear},
                                       {"Kp_memadd", Keys::Kp_memadd},
                                       {"Kp_memsubtract", Keys::Kp_memsubtract},
                                       {"Kp_memmultiply", Keys::Kp_memmultiply},
                                       {"Kp_memdivide", Keys::Kp_memdivide},
                                       {"Kp_plusminus", Keys::Kp_plusminus},
                                       {"Kp_clear", Keys::Kp_clear},
                                       {"Kp_clearentry", Keys::Kp_clearentry},
                                       {"Kp_binary", Keys::Kp_binary},
                                       {"Kp_octal", Keys::Kp_octal},
                                       {"Kp_decimal", Keys::Kp_decimal},
                                       {"Kp_hexadecimal", Keys::Kp_hexadecimal},

                                       {"Lctrl", Keys::Lctrl},
                                       {"Lshift", Keys::Lshift},
                                       {"Lalt", Keys::Lalt},
                                       {"Lgui", Keys::Lgui},
                                       {"Rctrl", Keys::Rctrl},
                                       {"Rshift", Keys::Rshift},
                                       {"Ralt", Keys::Ralt},
                                       {"Rgui", Keys::Rgui},

                                       {"Mode", Keys::Mode},

                                       {"Audionext", Keys::Audionext},
                                       {"Audioprev", Keys::Audioprev},
                                       {"Audiostop", Keys::Audiostop},
                                       {"Audioplay", Keys::Audioplay},
                                       {"Audiomute", Keys::Audiomute},
                                       {"Mediaselect", Keys::Mediaselect},
                                       {"Www", Keys::Www},
                                       {"Mail", Keys::Mail},
                                       {"Calculator", Keys::Calculator},
                                       {"Computer", Keys::Computer},
                                       {"Ac_search", Keys::Ac_search},
                                       {"Ac_home", Keys::Ac_home},
                                       {"Ac_back", Keys::Ac_back},
                                       {"Ac_forward", Keys::Ac_forward},
                                       {"Ac_stop", Keys::Ac_stop},
                                       {"Ac_refresh", Keys::Ac_refresh},
                                       {"Ac_bookmarks", Keys::Ac_bookmarks},

                                       {"Brightnessdown", Keys::Brightnessdown},
                                       {"Brightnessup", Keys::Brightnessup},
                                       {"Displayswitch", Keys::Displayswitch},
                                       {"Kbdillumtoggle", Keys::Kbdillumtoggle},
                                       {"Kbdillumdown", Keys::Kbdillumdown},
                                       {"Kbdillumup", Keys::Kbdillumup},
                                       {"Eject", Keys::Eject},
                                       {"Sleep", Keys::Sleep},
                                       {"App1", Keys::App1},
                                   });
    }
};  // namespace mag
