/**
 * @file tab_page_richtext.h
 * @brief Tab page hosting a wl::richedit control.
 *
 * Part of ModernWinApp.
 */

#pragma once
#include <Windows.h>
#include "winlamb/window_control.h"
#include "wrappers/richedit.h"

/**
 * @brief Tab page that fills its entire client area with a RichEdit50W control.
 *
 * Pre-loaded with sample text demonstrating the richedit wrapper capabilities.
 */
class TabPageRichText : public wl::window_control {
private:
    wl::richedit _edit;

public:
    /**
     * @brief Constructs the tab page and registers WM_CREATE / WM_SIZE handlers.
     */
    TabPageRichText() {
        this->setup.wndClassEx.lpszClassName = L"WL_RICHTEXT_PAGE";
        this->setup.wndClassEx.hbrBackground =
            reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

        this->on_message(WM_CREATE, [this](wl::params) -> LRESULT {
            RECT rc{};
            GetClientRect(this->hwnd(), &rc);
            this->_edit.create(this->hwnd(), 1,
                {0, 0}, {rc.right, rc.bottom});
            this->_edit.set_font(L"Consolas", 11);
            this->_edit.set_text(
                L"Welcome to ModernWinApp\r\n"
                L"==============================\r\n\r\n"
                L"This is a RichEdit50W control (MSFTEDIT_CLASS).\r\n\r\n"
                L"Wrapper features (wl::richedit):\r\n"
                L"  - set_text / get_text\r\n"
                L"  - append_text (EM_REPLACESEL)\r\n"
                L"  - set_font via CHARFORMAT2 / SCF_ALL\r\n"
                L"  - set_readonly\r\n"
                L"  - Full ES_MULTILINE with vertical scroll\r\n");
            return 0;
        });

        this->on_message(WM_SIZE, [this](wl::params p) -> LRESULT {
            const int cx = LOWORD(p.lParam);
            const int cy = HIWORD(p.lParam);
            if (this->_edit.hwnd()) {
                SetWindowPos(this->_edit.hwnd(), nullptr,
                    0, 0, cx, cy,
                    SWP_NOZORDER | SWP_NOACTIVATE);
            }
            return 0;
        });
    }

    TabPageRichText(TabPageRichText&&) = default;
    TabPageRichText& operator=(TabPageRichText&&) = default; ///< Move-only.
};
