//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_DETAIL_ALGORITHM_CONSOLE_UNICODE_GUARD_HPP
#define SMALL_DETAIL_ALGORITHM_CONSOLE_UNICODE_GUARD_HPP

/// \section This guard encapsulates the logic to temporarily set the Windows
/// console to UTF8 if:
/// - the platform is windows
/// - the input string requires unicode
/// - the ostream is the console
/// - the terminal doesn't currently support unicode
/// It might also need to temporarily change the console font if the input
/// string includes large code points

#include <iostream>

#if defined(_WIN32) && __has_include(<Windows.h>)
#    include <Windows.h>
#    undef small
#endif

namespace small {
    namespace detail {
        class console_unicode_guard
        {
        public:
            inline console_unicode_guard(
                std::ostream &os,
                size_t size,
                size_t codepoints,
                bool above_10000 = true) {
#if defined(_WIN32) && __has_include(<Windows.h>)
                const bool is_console
                    = &os == reinterpret_cast<std::ostream *>(&std::cout)
                      || &os == reinterpret_cast<std::ostream *>(&std::wcout);
                const bool requires_unicode = size != codepoints;
                prev_console_output_cp = GetConsoleOutputCP();
                requires_another_console_cp = is_console && requires_unicode
                                              && prev_console_output_cp
                                                     != CP_UTF8;
                if (requires_another_console_cp) {
                    SetConsoleOutputCP(CP_UTF8);
                }
                // If the highest codepoint is above U+10000, we also need to
                // change the default console font to one that supports these
                // characters if the current one can't support them yet.
                // Unfortunately, cmd.exe won't support high codepoints even if
                // the corresponding fonts support them. But this solves the
                // problem for the terminals, like most IDE terminals, and
                // future versions of cmd.exe.
                if (requires_unicode && above_10000) {
                    // Console handle
                    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
                    // Get current font
                    ZeroMemory(&prev_console_font, sizeof(prev_console_font));
                    prev_console_font.cbSize = sizeof(prev_console_font);
                    if (GetCurrentConsoleFontEx(
                            hStdOut,
                            false,
                            &prev_console_font)) {
                        // Check if current font supports unicode above 10000
                        // There's no simple heuristic to do that, be we do know
                        // 1) the default console font (consolas) do not support
                        // unicode above 10000 and 2) the user probably doesn't
                        // want another font if they explicitly chose that font.
                        requires_another_font = std::
                            wcscmp(prev_console_font.FaceName, L"Consolas");
                        if (requires_another_font) {
                            CONSOLE_FONT_INFOEX new_font;
                            ZeroMemory(&new_font, sizeof(new_font));
                            new_font.cbSize = sizeof(new_font);
                            lstrcpyW(new_font.FaceName, L"Lucida Console");
                            SetCurrentConsoleFontEx(hStdOut, false, &new_font);
                        }
                    }
                }
#else
                // Discard values
                (void) os;
                (void) size;
                (void) codepoints;
                (void) above_10000;
#endif
            }

            inline ~console_unicode_guard() {
#if defined(_WIN32) && __has_include(<Windows.h>)
                if (requires_another_console_cp) {
                    SetConsoleOutputCP(prev_console_output_cp);
                }
                if (requires_another_font) {
                    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
                    SetCurrentConsoleFontEx(hStdOut, false, &prev_console_font);
                }
#endif
            }

        private:
#if defined(_WIN32) && __has_include(<Windows.h>)
            bool requires_another_console_cp{ false };
            UINT prev_console_output_cp{ 0 };
            bool requires_another_font{ false };
            CONSOLE_FONT_INFOEX prev_console_font{};
#endif
        };

    } // namespace detail
} // namespace small

#endif // SMALL_DETAIL_ALGORITHM_CONSOLE_UNICODE_GUARD_HPP
