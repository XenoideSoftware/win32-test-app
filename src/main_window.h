#pragma once
#include <Windows.h>
#include <CommCtrl.h>
#include "winlamb/window_main.h"
#include "splitter_horizontal.h"
#include "splitter_vertical.h"

/**
 * @brief Root application window, rebuilt to test the vertical splitter.
 */
class MainWindow : public wl::window_main {
private:
    HWND              _hEditLeft = nullptr;
    HWND              _hEditRight = nullptr;
    splitter_horizontal _splitter;

    /**
     * @brief Repositions the splitter window natively triggering WM_SIZE cascade.
     */
    void _layout() noexcept {
        RECT rc;
        GetClientRect(this->hwnd(), &rc);
        if (!_splitter.hwnd()) return;

        // Note: We use SetWindowPos instead of defer_window_pos here!
        // EndDeferWindowPos triggers WM_SIZE synchronously. If we wrap this
        // in a defer_window_pos, the nested defer_window_pos inside the splitter's
        // WM_SIZE event will fail returning a null HDWP handle, leaving controls frozen.
        SetWindowPos(_splitter.hwnd(), HWND_BOTTOM, 0, 0, rc.right, rc.bottom, SWP_NOACTIVATE);
    }

public:
    MainWindow() {
        this->setup.wndClassEx.lpszClassName = L"MODERNWINAPP_MAIN";
        this->setup.title = L"Splitter Utility Test (Vertical anchor_second)";
        this->setup.style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
        this->setup.size  = {800, 600};

        this->on_message(WM_CREATE, [this](wl::params p) -> LRESULT {
            // 1. Initialize Splitter first so it is at the bottom of the Z-order
            _splitter.create(this, 103, {0, 0}, {0, 0});
            _splitter.set_resize_mode(splitter_resize_mode::anchor_first);
            // _splitter.set_split_pos(300); // Removed to test 50/50 default split

            // 2. Create Left Edit
            _hEditLeft = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"Left Edit Window...\r\nDrag the splitter to the right to resize.",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_CLIPSIBLINGS | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
                0, 0, 0, 0, this->hwnd(), reinterpret_cast<HMENU>(101), GetModuleHandleW(nullptr), nullptr);
            
            // 3. Create Right Edit
            _hEditRight = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"Right Edit (Anchored)....\r\nResize the App Window gently to watch me firmly hold absolute width!",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_CLIPSIBLINGS | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
                0, 0, 0, 0, this->hwnd(), reinterpret_cast<HMENU>(102), GetModuleHandleW(nullptr), nullptr);

            _splitter.set_windows(_hEditLeft, _hEditRight);

            // Set GUI Font
            HFONT hFont = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
            SendMessage(_hEditLeft, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), FALSE);
            SendMessage(_hEditRight, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), FALSE);

            // Initial Layout computation
            _layout();
            return 0;
        });

        // Track user resizing the main window frame
        this->on_message(WM_SIZE, [this](wl::params p) -> LRESULT {
            if (p.wParam != SIZE_MINIMIZED && _splitter.hwnd()) {
                _layout();
            }
            return 0;
        });
    }

    MainWindow(MainWindow&&) = default;
    MainWindow& operator=(MainWindow&&) = default;
};
