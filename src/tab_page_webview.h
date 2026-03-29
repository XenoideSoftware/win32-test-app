/**
 * @file tab_page_webview.h
 * @brief Tab page hosting a wl::webview panel.
 *
 * Part of ModernWinApp.
 */

#pragma once
#include <Windows.h>
#include "winlamb/window_control.h"
#include "wrappers/webview.h"

/**
 * @brief Tab page that fills its entire client area with a wl::webview panel.
 *
 * Navigates to a placeholder URL on creation. Replace wl::webview with a
 * real browser embedding (WebView2 or IWebBrowser2) for production use.
 */
class TabPageWebView : public wl::window_control {
private:
    wl::webview _webview;

public:
    /**
     * @brief Constructs the tab page and registers WM_CREATE / WM_SIZE handlers.
     */
    TabPageWebView() {
        this->setup.wndClassEx.lpszClassName = L"WL_WEBVIEW_PAGE";
        this->setup.wndClassEx.hbrBackground =
            reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

        this->on_message(WM_CREATE, [this](wl::params) -> LRESULT {
            RECT rc{};
            GetClientRect(this->hwnd(), &rc);
            this->_webview.create(this->hwnd(), 1,
                {0, 0}, {rc.right, rc.bottom});
            this->_webview.navigate(L"https://example.com");
            return 0;
        });

        this->on_message(WM_SIZE, [this](wl::params p) -> LRESULT {
            const int cx = LOWORD(p.lParam);
            const int cy = HIWORD(p.lParam);
            if (this->_webview.hwnd()) {
                SetWindowPos(this->_webview.hwnd(), nullptr,
                    0, 0, cx, cy,
                    SWP_NOZORDER | SWP_NOACTIVATE);
            }
            return 0;
        });
    }

    TabPageWebView(TabPageWebView&&) = default;
    TabPageWebView& operator=(TabPageWebView&&) = default; ///< Move-only.
};
