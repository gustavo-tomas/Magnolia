#include "private/key_mappings.hpp"

#include "SDL_keycode.h"
#include "SDL_mouse.h"

namespace mag
{
    // clang-format off
    
    i32 KeycodeMapper::to_SDL_button(const Button button)
    {
        switch (button)
        {
            case Button::Left:    return SDL_BUTTON_LEFT;
            case Button::Middle:  return SDL_BUTTON_MIDDLE;
            case Button::Right:   return SDL_BUTTON_RIGHT;
            case Button::X1:      return SDL_BUTTON_X1;
            case Button::X2:      return SDL_BUTTON_X2;
            case Button::Unknown:
            default:              return 0;
        }
    }

    Button KeycodeMapper::from_SDL_button(const i32 sdl_button)
    {
        switch (sdl_button)
        {
            case SDL_BUTTON_LEFT:   return Button::Left;
            case SDL_BUTTON_MIDDLE: return Button::Middle;
            case SDL_BUTTON_RIGHT:  return Button::Right;
            case SDL_BUTTON_X1:     return Button::X1;
            case SDL_BUTTON_X2:     return Button::X2;
            default:                return Button::Unknown;
        }
    }

    SDL_Keycode KeycodeMapper::to_SDL_keycode(Key key)
    {
        switch (key)
        {
            case Key::Enter: return SDLK_RETURN;
            case Key::Escape: return SDLK_ESCAPE;
            case Key::Backspace: return SDLK_BACKSPACE;
            case Key::Tab: return SDLK_TAB;
            case Key::Space: return SDLK_SPACE;
            case Key::Exclaim: return SDLK_EXCLAIM;
            case Key::Quotedbl: return SDLK_QUOTEDBL;
            case Key::Hash: return SDLK_HASH;
            case Key::Percent: return SDLK_PERCENT;
            case Key::Dollar: return SDLK_DOLLAR;
            case Key::Ampersand: return SDLK_AMPERSAND;
            case Key::Quote: return SDLK_QUOTE;
            case Key::Leftparen: return SDLK_LEFTPAREN;
            case Key::Rightparen: return SDLK_RIGHTPAREN;
            case Key::Asterisk: return SDLK_ASTERISK;
            case Key::Plus: return SDLK_PLUS;
            case Key::Comma: return SDLK_COMMA;
            case Key::Minus: return SDLK_MINUS;
            case Key::Period: return SDLK_PERIOD;
            case Key::Slash: return SDLK_SLASH;
            case Key::_0: return SDLK_0;
            case Key::_1: return SDLK_1;
            case Key::_2: return SDLK_2;
            case Key::_3: return SDLK_3;
            case Key::_4: return SDLK_4;
            case Key::_5: return SDLK_5;
            case Key::_6: return SDLK_6;
            case Key::_7: return SDLK_7;
            case Key::_8: return SDLK_8;
            case Key::_9: return SDLK_9;
            case Key::Colon: return SDLK_COLON;
            case Key::Semicolon: return SDLK_SEMICOLON;
            case Key::Less: return SDLK_LESS;
            case Key::Equals: return SDLK_EQUALS;
            case Key::Greater: return SDLK_GREATER;
            case Key::Question: return SDLK_QUESTION;
            case Key::At: return SDLK_AT;

            /*
                Skip uppercase letters
            */

            case Key::Leftbracket: return SDLK_LEFTBRACKET;
            case Key::Backslash: return SDLK_BACKSLASH;
            case Key::Rightbracket: return SDLK_RIGHTBRACKET;
            case Key::Caret: return SDLK_CARET;
            case Key::Underscore: return SDLK_UNDERSCORE;
            case Key::Backquote: return SDLK_BACKQUOTE;
            case Key::a: return SDLK_a;
            case Key::b: return SDLK_b;
            case Key::c: return SDLK_c;
            case Key::d: return SDLK_d;
            case Key::e: return SDLK_e;
            case Key::f: return SDLK_f;
            case Key::g: return SDLK_g;
            case Key::h: return SDLK_h;
            case Key::i: return SDLK_i;
            case Key::j: return SDLK_j;
            case Key::k: return SDLK_k;
            case Key::l: return SDLK_l;
            case Key::m: return SDLK_m;
            case Key::n: return SDLK_n;
            case Key::o: return SDLK_o;
            case Key::p: return SDLK_p;
            case Key::q: return SDLK_q;
            case Key::r: return SDLK_r;
            case Key::s: return SDLK_s;
            case Key::t: return SDLK_t;
            case Key::u: return SDLK_u;
            case Key::v: return SDLK_v;
            case Key::w: return SDLK_w;
            case Key::x: return SDLK_x;
            case Key::y: return SDLK_y;
            case Key::z: return SDLK_z;

            case Key::Capslock: return SDLK_CAPSLOCK;

            case Key::F1: return SDLK_F1;
            case Key::F2: return SDLK_F2;
            case Key::F3: return SDLK_F3;
            case Key::F4: return SDLK_F4;
            case Key::F5: return SDLK_F5;
            case Key::F6: return SDLK_F6;
            case Key::F7: return SDLK_F7;
            case Key::F8: return SDLK_F8;
            case Key::F9: return SDLK_F9;
            case Key::F10: return SDLK_F10;
            case Key::F11: return SDLK_F11;
            case Key::F12: return SDLK_F12;

            case Key::Printscreen: return SDLK_PRINTSCREEN;
            case Key::Scrolllock: return SDLK_SCROLLLOCK;
            case Key::Pause: return SDLK_PAUSE;
            case Key::Insert: return SDLK_INSERT;
            case Key::Home: return SDLK_HOME;
            case Key::Pageup: return SDLK_PAGEUP;
            case Key::Delete: return SDLK_DELETE;
            case Key::End: return SDLK_END;
            case Key::Pagedown: return SDLK_PAGEDOWN;
            case Key::Right: return SDLK_RIGHT;
            case Key::Left: return SDLK_LEFT;
            case Key::Down: return SDLK_DOWN;
            case Key::Up: return SDLK_UP;

            case Key::Numlockclear: return SDLK_NUMLOCKCLEAR;
            case Key::Kp_divide: return SDLK_KP_DIVIDE;
            case Key::Kp_multiply: return SDLK_KP_MULTIPLY;
            case Key::Kp_minus: return SDLK_KP_MINUS;
            case Key::Kp_plus: return SDLK_KP_PLUS;
            case Key::Kp_enter: return SDLK_KP_ENTER;
            case Key::Kp_1: return SDLK_KP_1;
            case Key::Kp_2: return SDLK_KP_2;
            case Key::Kp_3: return SDLK_KP_3;
            case Key::Kp_4: return SDLK_KP_4;
            case Key::Kp_5: return SDLK_KP_5;
            case Key::Kp_6: return SDLK_KP_6;
            case Key::Kp_7: return SDLK_KP_7;
            case Key::Kp_8: return SDLK_KP_8;
            case Key::Kp_9: return SDLK_KP_9;
            case Key::Kp_0: return SDLK_KP_0;
            case Key::Kp_period: return SDLK_KP_PERIOD;

            case Key::Application: return SDLK_APPLICATION;
            case Key::Power: return SDLK_POWER;
            case Key::Kp_equals: return SDLK_KP_EQUALS;
            case Key::F13: return SDLK_F13;
            case Key::F14: return SDLK_F14;
            case Key::F15: return SDLK_F15;
            case Key::F16: return SDLK_F16;
            case Key::F17: return SDLK_F17;
            case Key::F18: return SDLK_F18;
            case Key::F19: return SDLK_F19;
            case Key::F20: return SDLK_F20;
            case Key::F21: return SDLK_F21;
            case Key::F22: return SDLK_F22;
            case Key::F23: return SDLK_F23;
            case Key::F24: return SDLK_F24;
            case Key::Execute: return SDLK_EXECUTE;
            case Key::Help: return SDLK_HELP;
            case Key::Menu: return SDLK_MENU;
            case Key::Select: return SDLK_SELECT;
            case Key::Stop: return SDLK_STOP;
            case Key::Again: return SDLK_AGAIN;
            case Key::Undo: return SDLK_UNDO;
            case Key::Cut: return SDLK_CUT;
            case Key::Copy: return SDLK_COPY;
            case Key::Paste: return SDLK_PASTE;
            case Key::Find: return SDLK_FIND;
            case Key::Mute: return SDLK_MUTE;
            case Key::Volumeup: return SDLK_VOLUMEUP;
            case Key::Volumedown: return SDLK_VOLUMEDOWN;
            case Key::Kp_comma: return SDLK_KP_COMMA;
            case Key::Kp_equalsas400: return SDLK_KP_EQUALSAS400;

            case Key::Alterase: return SDLK_ALTERASE;
            case Key::Sysreq: return SDLK_SYSREQ;
            case Key::Cancel: return SDLK_CANCEL;
            case Key::Clear: return SDLK_CLEAR;
            case Key::Prior: return SDLK_PRIOR;
            case Key::Return2: return SDLK_RETURN2;
            case Key::Separator: return SDLK_SEPARATOR;
            case Key::Out: return SDLK_OUT;
            case Key::Oper: return SDLK_OPER;
            case Key::Clearagain: return SDLK_CLEARAGAIN;
            case Key::Crsel: return SDLK_CRSEL;
            case Key::Exsel: return SDLK_EXSEL;

            case Key::Kp_00: return SDLK_KP_00;
            case Key::Kp_000: return SDLK_KP_000;
            case Key::Thousandsseparator: return SDLK_THOUSANDSSEPARATOR;
            case Key::Decimalseparator: return SDLK_DECIMALSEPARATOR;
            case Key::Currencyunit: return SDLK_CURRENCYUNIT;
            case Key::Currencysubunit: return SDLK_CURRENCYSUBUNIT;
            case Key::Kp_leftparen: return SDLK_KP_LEFTPAREN;
            case Key::Kp_rightparen: return SDLK_KP_RIGHTPAREN;
            case Key::Kp_leftbrace: return SDLK_KP_LEFTBRACE;
            case Key::Kp_rightbrace: return SDLK_KP_RIGHTBRACE;
            case Key::Kp_tab: return SDLK_KP_TAB;
            case Key::Kp_backspace: return SDLK_KP_BACKSPACE;
            case Key::Kp_a: return SDLK_KP_A;
            case Key::Kp_b: return SDLK_KP_B;
            case Key::Kp_c: return SDLK_KP_C;
            case Key::Kp_d: return SDLK_KP_D;
            case Key::Kp_e: return SDLK_KP_E;
            case Key::Kp_f: return SDLK_KP_F;
            case Key::Kp_xor: return SDLK_KP_XOR;
            case Key::Kp_power: return SDLK_KP_POWER;
            case Key::Kp_percent: return SDLK_KP_PERCENT;
            case Key::Kp_less: return SDLK_KP_LESS;
            case Key::Kp_greater: return SDLK_KP_GREATER;
            case Key::Kp_ampersand: return SDLK_KP_AMPERSAND;
            case Key::Kp_dblampersand: return SDLK_KP_DBLAMPERSAND;
            case Key::Kp_verticalbar: return SDLK_KP_VERTICALBAR;
            case Key::Kp_dblverticalbar: return SDLK_KP_DBLVERTICALBAR;
            case Key::Kp_colon: return SDLK_KP_COLON;
            case Key::Kp_hash: return SDLK_KP_HASH;
            case Key::Kp_space: return SDLK_KP_SPACE;
            case Key::Kp_at: return SDLK_KP_AT;
            case Key::Kp_exclam: return SDLK_KP_EXCLAM;
            case Key::Kp_memstore: return SDLK_KP_MEMSTORE;
            case Key::Kp_memrecall: return SDLK_KP_MEMRECALL;
            case Key::Kp_memclear: return SDLK_KP_MEMCLEAR;
            case Key::Kp_memadd: return SDLK_KP_MEMADD;
            case Key::Kp_memsubtract: return SDLK_KP_MEMSUBTRACT;
            case Key::Kp_memmultiply: return SDLK_KP_MEMMULTIPLY;
            case Key::Kp_memdivide: return SDLK_KP_MEMDIVIDE;
            case Key::Kp_plusminus: return SDLK_KP_PLUSMINUS;
            case Key::Kp_clear: return SDLK_KP_CLEAR;
            case Key::Kp_clearentry: return SDLK_KP_CLEARENTRY;
            case Key::Kp_binary: return SDLK_KP_BINARY;
            case Key::Kp_octal: return SDLK_KP_OCTAL;
            case Key::Kp_decimal: return SDLK_KP_DECIMAL;
            case Key::Kp_hexadecimal: return SDLK_KP_HEXADECIMAL;

            case Key::Lctrl: return SDLK_LCTRL;
            case Key::Lshift: return SDLK_LSHIFT;
            case Key::Lalt: return SDLK_LALT;
            case Key::Lgui: return SDLK_LGUI;
            case Key::Rctrl: return SDLK_RCTRL;
            case Key::Rshift: return SDLK_RSHIFT;
            case Key::Ralt: return SDLK_RALT;
            case Key::Rgui: return SDLK_RGUI;

            case Key::Mode: return SDLK_MODE;

            case Key::Audionext: return SDLK_AUDIONEXT;
            case Key::Audioprev: return SDLK_AUDIOPREV;
            case Key::Audiostop: return SDLK_AUDIOSTOP;
            case Key::Audioplay: return SDLK_AUDIOPLAY;
            case Key::Audiomute: return SDLK_AUDIOMUTE;
            case Key::Mediaselect: return SDLK_MEDIASELECT;
            case Key::Www: return SDLK_WWW;
            case Key::Mail: return SDLK_MAIL;
            case Key::Calculator: return SDLK_CALCULATOR;
            case Key::Computer: return SDLK_COMPUTER;
            case Key::Ac_search: return SDLK_AC_SEARCH;
            case Key::Ac_home: return SDLK_AC_HOME;
            case Key::Ac_back: return SDLK_AC_BACK;
            case Key::Ac_forward: return SDLK_AC_FORWARD;
            case Key::Ac_stop: return SDLK_AC_STOP;
            case Key::Ac_refresh: return SDLK_AC_REFRESH;
            case Key::Ac_bookmarks: return SDLK_AC_BOOKMARKS;

            case Key::Brightnessdown: return SDLK_BRIGHTNESSDOWN;
            case Key::Brightnessup: return SDLK_BRIGHTNESSUP;
            case Key::Displayswitch: return SDLK_DISPLAYSWITCH;
            case Key::Kbdillumtoggle: return SDLK_KBDILLUMTOGGLE;
            case Key::Kbdillumdown: return SDLK_KBDILLUMDOWN;
            case Key::Kbdillumup: return SDLK_KBDILLUMUP;
            case Key::Eject: return SDLK_EJECT;
            case Key::Sleep: return SDLK_SLEEP;
            case Key::App1: return SDLK_APP1;
            case Key::Unknown: 
            default: return SDLK_UNKNOWN;
        }
    }

    Key KeycodeMapper::from_SDL_keycode(SDL_Keycode sdl_keycode)
    {
        switch (sdl_keycode)
        {
            case SDLK_RETURN: return Key::Enter;
            case SDLK_ESCAPE: return Key::Escape;
            case SDLK_BACKSPACE: return Key::Backspace;
            case SDLK_TAB: return Key::Tab;
            case SDLK_SPACE: return Key::Space;
            case SDLK_EXCLAIM: return Key::Exclaim;
            case SDLK_QUOTEDBL: return Key::Quotedbl;
            case SDLK_HASH: return Key::Hash;
            case SDLK_PERCENT: return Key::Percent;
            case SDLK_DOLLAR: return Key::Dollar;
            case SDLK_AMPERSAND: return Key::Ampersand;
            case SDLK_QUOTE: return Key::Quote;
            case SDLK_LEFTPAREN: return Key::Leftparen;
            case SDLK_RIGHTPAREN: return Key::Rightparen;
            case SDLK_ASTERISK: return Key::Asterisk;
            case SDLK_PLUS: return Key::Plus;
            case SDLK_COMMA: return Key::Comma;
            case SDLK_MINUS: return Key::Minus;
            case SDLK_PERIOD: return Key::Period;
            case SDLK_SLASH: return Key::Slash;
            case SDLK_0: return Key::_0;
            case SDLK_1: return Key::_1;
            case SDLK_2: return Key::_2;
            case SDLK_3: return Key::_3;
            case SDLK_4: return Key::_4;
            case SDLK_5: return Key::_5;
            case SDLK_6: return Key::_6;
            case SDLK_7: return Key::_7;
            case SDLK_8: return Key::_8;
            case SDLK_9: return Key::_9;
            case SDLK_COLON: return Key::Colon;
            case SDLK_SEMICOLON: return Key::Semicolon;
            case SDLK_LESS: return Key::Less;
            case SDLK_EQUALS: return Key::Equals;
            case SDLK_GREATER: return Key::Greater;
            case SDLK_QUESTION: return Key::Question;
            case SDLK_AT: return Key::At;

            /*
                Skip uppercase letters
            */

            case SDLK_LEFTBRACKET: return Key::Leftbracket;
            case SDLK_BACKSLASH: return Key::Backslash;
            case SDLK_RIGHTBRACKET: return Key::Rightbracket;
            case SDLK_CARET: return Key::Caret;
            case SDLK_UNDERSCORE: return Key::Underscore;
            case SDLK_BACKQUOTE: return Key::Backquote;
            case SDLK_a: return Key::a;
            case SDLK_b: return Key::b;
            case SDLK_c: return Key::c;
            case SDLK_d: return Key::d;
            case SDLK_e: return Key::e;
            case SDLK_f: return Key::f;
            case SDLK_g: return Key::g;
            case SDLK_h: return Key::h;
            case SDLK_i: return Key::i;
            case SDLK_j: return Key::j;
            case SDLK_k: return Key::k;
            case SDLK_l: return Key::l;
            case SDLK_m: return Key::m;
            case SDLK_n: return Key::n;
            case SDLK_o: return Key::o;
            case SDLK_p: return Key::p;
            case SDLK_q: return Key::q;
            case SDLK_r: return Key::r;
            case SDLK_s: return Key::s;
            case SDLK_t: return Key::t;
            case SDLK_u: return Key::u;
            case SDLK_v: return Key::v;
            case SDLK_w: return Key::w;
            case SDLK_x: return Key::x;
            case SDLK_y: return Key::y;
            case SDLK_z: return Key::z;

            case SDLK_CAPSLOCK: return Key::Capslock;

            case SDLK_F1: return Key::F1;
            case SDLK_F2: return Key::F2;
            case SDLK_F3: return Key::F3;
            case SDLK_F4: return Key::F4;
            case SDLK_F5: return Key::F5;
            case SDLK_F6: return Key::F6;
            case SDLK_F7: return Key::F7;
            case SDLK_F8: return Key::F8;
            case SDLK_F9: return Key::F9;
            case SDLK_F10: return Key::F10;
            case SDLK_F11: return Key::F11;
            case SDLK_F12: return Key::F12;

            case SDLK_PRINTSCREEN: return Key::Printscreen;
            case SDLK_SCROLLLOCK: return Key::Scrolllock;
            case SDLK_PAUSE: return Key::Pause;
            case SDLK_INSERT: return Key::Insert;
            case SDLK_HOME: return Key::Home;
            case SDLK_PAGEUP: return Key::Pageup;
            case SDLK_DELETE: return Key::Delete;
            case SDLK_END: return Key::End;
            case SDLK_PAGEDOWN: return Key::Pagedown;
            case SDLK_RIGHT: return Key::Right;
            case SDLK_LEFT: return Key::Left;
            case SDLK_DOWN: return Key::Down;
            case SDLK_UP: return Key::Up;

            case SDLK_NUMLOCKCLEAR: return Key::Numlockclear;
            case SDLK_KP_DIVIDE: return Key::Kp_divide;
            case SDLK_KP_MULTIPLY: return Key::Kp_multiply;
            case SDLK_KP_MINUS: return Key::Kp_minus;
            case SDLK_KP_PLUS: return Key::Kp_plus;
            case SDLK_KP_ENTER: return Key::Kp_enter;
            case SDLK_KP_1: return Key::Kp_1;
            case SDLK_KP_2: return Key::Kp_2;
            case SDLK_KP_3: return Key::Kp_3;
            case SDLK_KP_4: return Key::Kp_4;
            case SDLK_KP_5: return Key::Kp_5;
            case SDLK_KP_6: return Key::Kp_6;
            case SDLK_KP_7: return Key::Kp_7;
            case SDLK_KP_8: return Key::Kp_8;
            case SDLK_KP_9: return Key::Kp_9;
            case SDLK_KP_0: return Key::Kp_0;
            case SDLK_KP_PERIOD: return Key::Kp_period;

            case SDLK_APPLICATION: return Key::Application;
            case SDLK_POWER: return Key::Power;
            case SDLK_KP_EQUALS: return Key::Kp_equals;
            case SDLK_F13: return Key::F13;
            case SDLK_F14: return Key::F14;
            case SDLK_F15: return Key::F15;
            case SDLK_F16: return Key::F16;
            case SDLK_F17: return Key::F17;
            case SDLK_F18: return Key::F18;
            case SDLK_F19: return Key::F19;
            case SDLK_F20: return Key::F20;
            case SDLK_F21: return Key::F21;
            case SDLK_F22: return Key::F22;
            case SDLK_F23: return Key::F23;
            case SDLK_F24: return Key::F24;
            case SDLK_EXECUTE: return Key::Execute;
            case SDLK_HELP: return Key::Help;
            case SDLK_MENU: return Key::Menu;
            case SDLK_SELECT: return Key::Select;
            case SDLK_STOP: return Key::Stop;
            case SDLK_AGAIN: return Key::Again;
            case SDLK_UNDO: return Key::Undo;
            case SDLK_CUT: return Key::Cut;
            case SDLK_COPY: return Key::Copy;
            case SDLK_PASTE: return Key::Paste;
            case SDLK_FIND: return Key::Find;
            case SDLK_MUTE: return Key::Mute;
            case SDLK_VOLUMEUP: return Key::Volumeup;
            case SDLK_VOLUMEDOWN: return Key::Volumedown;
            case SDLK_KP_COMMA: return Key::Kp_comma;
            case SDLK_KP_EQUALSAS400: return Key::Kp_equalsas400;

            case SDLK_ALTERASE: return Key::Alterase;
            case SDLK_SYSREQ: return Key::Sysreq;
            case SDLK_CANCEL: return Key::Cancel;
            case SDLK_CLEAR: return Key::Clear;
            case SDLK_PRIOR: return Key::Prior;
            case SDLK_RETURN2: return Key::Return2;
            case SDLK_SEPARATOR: return Key::Separator;
            case SDLK_OUT: return Key::Out;
            case SDLK_OPER: return Key::Oper;
            case SDLK_CLEARAGAIN: return Key::Clearagain;
            case SDLK_CRSEL: return Key::Crsel;
            case SDLK_EXSEL: return Key::Exsel;

            case SDLK_KP_00: return Key::Kp_00;
            case SDLK_KP_000: return Key::Kp_000;
            case SDLK_THOUSANDSSEPARATOR: return Key::Thousandsseparator;
            case SDLK_DECIMALSEPARATOR: return Key::Decimalseparator;
            case SDLK_CURRENCYUNIT: return Key::Currencyunit;
            case SDLK_CURRENCYSUBUNIT: return Key::Currencysubunit;
            case SDLK_KP_LEFTPAREN: return Key::Kp_leftparen;
            case SDLK_KP_RIGHTPAREN: return Key::Kp_rightparen;
            case SDLK_KP_LEFTBRACE: return Key::Kp_leftbrace;
            case SDLK_KP_RIGHTBRACE: return Key::Kp_rightbrace;
            case SDLK_KP_TAB: return Key::Kp_tab;
            case SDLK_KP_BACKSPACE: return Key::Kp_backspace;
            case SDLK_KP_A: return Key::Kp_a;
            case SDLK_KP_B: return Key::Kp_b;
            case SDLK_KP_C: return Key::Kp_c;
            case SDLK_KP_D: return Key::Kp_d;
            case SDLK_KP_E: return Key::Kp_e;
            case SDLK_KP_F: return Key::Kp_f;
            case SDLK_KP_XOR: return Key::Kp_xor;
            case SDLK_KP_POWER: return Key::Kp_power;
            case SDLK_KP_PERCENT: return Key::Kp_percent;
            case SDLK_KP_LESS: return Key::Kp_less;
            case SDLK_KP_GREATER: return Key::Kp_greater;
            case SDLK_KP_AMPERSAND: return Key::Kp_ampersand;
            case SDLK_KP_DBLAMPERSAND: return Key::Kp_dblampersand;
            case SDLK_KP_VERTICALBAR: return Key::Kp_verticalbar;
            case SDLK_KP_DBLVERTICALBAR: return Key::Kp_dblverticalbar;
            case SDLK_KP_COLON: return Key::Kp_colon;
            case SDLK_KP_HASH: return Key::Kp_hash;
            case SDLK_KP_SPACE: return Key::Kp_space;
            case SDLK_KP_AT: return Key::Kp_at;
            case SDLK_KP_EXCLAM: return Key::Kp_exclam;
            case SDLK_KP_MEMSTORE: return Key::Kp_memstore;
            case SDLK_KP_MEMRECALL: return Key::Kp_memrecall;
            case SDLK_KP_MEMCLEAR: return Key::Kp_memclear;
            case SDLK_KP_MEMADD: return Key::Kp_memadd;
            case SDLK_KP_MEMSUBTRACT: return Key::Kp_memsubtract;
            case SDLK_KP_MEMMULTIPLY: return Key::Kp_memmultiply;
            case SDLK_KP_MEMDIVIDE: return Key::Kp_memdivide;
            case SDLK_KP_PLUSMINUS: return Key::Kp_plusminus;
            case SDLK_KP_CLEAR: return Key::Kp_clear;
            case SDLK_KP_CLEARENTRY: return Key::Kp_clearentry;
            case SDLK_KP_BINARY: return Key::Kp_binary;
            case SDLK_KP_OCTAL: return Key::Kp_octal;
            case SDLK_KP_DECIMAL: return Key::Kp_decimal;
            case SDLK_KP_HEXADECIMAL: return Key::Kp_hexadecimal;

            case SDLK_LCTRL: return Key::Lctrl;
            case SDLK_LSHIFT: return Key::Lshift;
            case SDLK_LALT: return Key::Lalt;
            case SDLK_LGUI: return Key::Lgui;
            case SDLK_RCTRL: return Key::Rctrl;
            case SDLK_RSHIFT: return Key::Rshift;
            case SDLK_RALT: return Key::Ralt;
            case SDLK_RGUI: return Key::Rgui;

            case SDLK_MODE: return Key::Mode;

            case SDLK_AUDIONEXT: return Key::Audionext;
            case SDLK_AUDIOPREV: return Key::Audioprev;
            case SDLK_AUDIOSTOP: return Key::Audiostop;
            case SDLK_AUDIOPLAY: return Key::Audioplay;
            case SDLK_AUDIOMUTE: return Key::Audiomute;
            case SDLK_MEDIASELECT: return Key::Mediaselect;
            case SDLK_WWW: return Key::Www;
            case SDLK_MAIL: return Key::Mail;
            case SDLK_CALCULATOR: return Key::Calculator;
            case SDLK_COMPUTER: return Key::Computer;
            case SDLK_AC_SEARCH: return Key::Ac_search;
            case SDLK_AC_HOME: return Key::Ac_home;
            case SDLK_AC_BACK: return Key::Ac_back;
            case SDLK_AC_FORWARD: return Key::Ac_forward;
            case SDLK_AC_STOP: return Key::Ac_stop;
            case SDLK_AC_REFRESH: return Key::Ac_refresh;
            case SDLK_AC_BOOKMARKS: return Key::Ac_bookmarks;

            case SDLK_BRIGHTNESSDOWN: return Key::Brightnessdown;
            case SDLK_BRIGHTNESSUP: return Key::Brightnessup;
            case SDLK_DISPLAYSWITCH: return Key::Displayswitch;
            case SDLK_KBDILLUMTOGGLE: return Key::Kbdillumtoggle;
            case SDLK_KBDILLUMDOWN: return Key::Kbdillumdown;
            case SDLK_KBDILLUMUP: return Key::Kbdillumup;
            case SDLK_EJECT: return Key::Eject;
            case SDLK_SLEEP: return Key::Sleep;
            case SDLK_APP1: return Key::App1;
            case SDLK_UNKNOWN:
            default: return Key::Unknown;
        }
    }

    // clang-format on
};  // namespace mag
