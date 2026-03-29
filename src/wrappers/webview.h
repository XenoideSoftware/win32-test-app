/**
 * @file webview.h
 * @brief Mock browser-style panel built on wl::window_control.
 *
 * Part of ModernWinApp — follows the WinLamb header-only, move-only pattern.
 */

#pragma once
#include <string>
#include <Windows.h>
#include "winlamb/window_control.h"
#include "winlamb/textbox.h"

namespace wl {

/**
 * @brief Mock browser-style embedded web panel.
 *
 * Displays a read-only URL bar and a placeholder content area. Provides the
 * structural API of an embedded browser — navigate(), get_url() — without
 * requiring an external browser runtime.
 *
 * @par Replacing with a real browser engine
 * Substitute the WM_CREATE body with one of the following:
 *
 * **Microsoft WebView2 (recommended):**
 * @code
 * #include <WebView2.h>
 * // In WM_CREATE: call CreateCoreWebView2EnvironmentWithOptions(...) then
 * // ICoreWebView2Environment::CreateCoreWebView2Controller(hwnd(), ...).
 * // See https://docs.microsoft.com/en-us/microsoft-edge/webview2/
 * @endcode
 *
 * **Legacy IWebBrowser2 (COM in-place activation):**
 * @code
 * #include <SHDocVw.h>   // IWebBrowser2
 * #include <AtlBase.h>   // ATL-based hosting via AtlAxCreateControl()
 * // Or implement IOleClientSite + IOleInPlaceSite manually and call
 * // CoCreateInstance(CLSID_WebBrowser) + IOleObject::DoVerb(OLEIVERB_INPLACEACTIVATE).
 * @endcode
 */
class webview : public window_control {
private:
    wl::textbox  _urlBar;
    std::wstring _currentUrl;

    static constexpr int URL_BAR_HEIGHT = 26; ///< Height of the URL bar in pixels.
    static constexpr int IDC_URLBAR     = 1;  ///< Child ID for the URL textbox.

public:
    /**
     * @brief Constructs the webview and registers its message handlers.
     *
     * The unique window class name `WL_WEBVIEW` is set here so that
     * wl::window_control::create() can register it before the HWND is created.
     */
    webview() {
        this->setup.wndClassEx.lpszClassName = L"WL_WEBVIEW";
        this->setup.wndClassEx.hbrBackground =
            reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

        this->on_message(WM_CREATE, [this](wl::params) -> LRESULT {
            RECT rc{};
            GetClientRect(this->hwnd(), &rc);
            this->_urlBar.create(this->hwnd(), IDC_URLBAR,
                wl::textbox::type::NORMAL,
                {0, 0}, rc.right, URL_BAR_HEIGHT);
            SendMessageW(this->_urlBar.hwnd(), EM_SETREADONLY, TRUE, 0);
            return 0;
        });

        this->on_message(WM_SIZE, [this](wl::params p) -> LRESULT {
            const int cx = LOWORD(p.lParam);
            if (this->_urlBar.hwnd()) {
                SetWindowPos(this->_urlBar.hwnd(), nullptr,
                    0, 0, cx, URL_BAR_HEIGHT,
                    SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
            }
            return 0;
        });

        this->on_message(WM_PAINT, [this](wl::params) -> LRESULT {
            PAINTSTRUCT ps{};
            HDC hdc = BeginPaint(this->hwnd(), &ps);

            RECT rc{};
            GetClientRect(this->hwnd(), &rc);

            // Thin separator line below the URL bar
            RECT lineRc{ rc.left, URL_BAR_HEIGHT - 1,
                         rc.right, URL_BAR_HEIGHT };
            FillRect(hdc, &lineRc,
                reinterpret_cast<HBRUSH>(COLOR_BTNSHADOW + 1));

            // Content placeholder area
            RECT contentRc{ rc.left, URL_BAR_HEIGHT, rc.right, rc.bottom };
            FillRect(hdc, &contentRc,
                reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));

            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, GetSysColor(COLOR_GRAYTEXT));
            DrawTextW(hdc,
                L"[ Web Content Placeholder ]\r\n\r\n"
                L"Replace wl::webview with WebView2\r\nor IWebBrowser2 hosting.",
                -1, &contentRc,
                DT_CENTER | DT_VCENTER | DT_WORDBREAK);

            EndPaint(this->hwnd(), &ps);
            return 0;
        });
    }

    webview(webview&&) = default;
    webview& operator=(webview&&) = default; ///< Move-only.

    /**
     * @brief Navigates to the given URL, updating the URL bar display.
     *
     * In this placeholder implementation the URL is stored and displayed in the
     * read-only textbox. Replace with `pWebBrowser->Navigate(url, ...)` or the
     * WebView2 equivalent.
     *
     * @param url  Target URL (e.g. `L"https://example.com"`).
     */
    webview& navigate(const std::wstring& url) {
        this->_currentUrl = url;
        if (this->_urlBar.hwnd()) {
            SendMessageW(this->_urlBar.hwnd(), WM_SETTEXT, 0,
                reinterpret_cast<LPARAM>(url.c_str()));
        }
        if (this->hwnd()) {
            InvalidateRect(this->hwnd(), nullptr, TRUE);
        }
        return *this;
    }

    /**
     * @brief Returns the most recently navigated-to URL.
     */
    const std::wstring& get_url() const noexcept {
        return this->_currentUrl;
    }
};

} // namespace wl
