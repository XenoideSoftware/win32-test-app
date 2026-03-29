#pragma once
#include <Windows.h>
#include <CommCtrl.h>
#include "winlamb/window_main.h"
#include "splitter_horizontal.h"
#include "splitter_vertical.h"

/**
 * @brief Root application window, rebuilt to test the horizontal splitter.
 */
class MainWindow : public wl::window_main {
private:
    HWND                _hEditTop = nullptr;
    HWND                _hEditBottom = nullptr;
    splitter_horizontal _splitter;

    int _splitterThickness = 6;
    int _defaultTopHeight = 200;

    /**
     * @brief Repositions the top edit, splitter, and bottom edit controls.
     */
    void _layout() noexcept {
        RECT rc;
        GetClientRect(this->hwnd(), &rc);
        if (!_hEditTop || !_hEditBottom || !_splitter.hwnd()) return;

        int w = rc.right;
        int h = rc.bottom;
        
        // Use the current height of the top window as the splitter anchor
        RECT rcTop;
        GetWindowRect(_hEditTop, &rcTop);
        int topHeight = rcTop.bottom - rcTop.top;
        if (topHeight == 0) topHeight = _defaultTopHeight; // Initial layout

        // Enforce valid Y position for the splitter within bounds
        if (topHeight > h - _splitterThickness) topHeight = h - _splitterThickness;
        if (topHeight < 0) topHeight = 0;

        HDWP hdwp = BeginDeferWindowPos(3);
        hdwp = DeferWindowPos(hdwp, _hEditTop, nullptr, 0, 0, w, topHeight, SWP_NOZORDER | SWP_NOACTIVATE);
        hdwp = DeferWindowPos(hdwp, _splitter.hwnd(), nullptr, 0, topHeight, w, _splitterThickness, SWP_NOZORDER | SWP_NOACTIVATE);
        hdwp = DeferWindowPos(hdwp, _hEditBottom, nullptr, 0, topHeight + _splitterThickness, w, h - topHeight - _splitterThickness, SWP_NOZORDER | SWP_NOACTIVATE);
        EndDeferWindowPos(hdwp);
    }

public:
    MainWindow() {
        this->setup.wndClassEx.lpszClassName = L"MODERNWINAPP_MAIN";
        this->setup.title = L"Splitter Utility Test";
        this->setup.style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
        this->setup.size  = {800, 600};

        this->on_message(WM_CREATE, [this](wl::params p) -> LRESULT {
            // Create Top Edit
            _hEditTop = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"Top Edit Window...\r\nDrag the splitter below to resize.",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
                0, 0, 0, 0, this->hwnd(), reinterpret_cast<HMENU>(101), GetModuleHandleW(nullptr), nullptr);
            
            // Create Bottom Edit
            _hEditBottom = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"Bottom Edit Window...\r\nTesting WinLamb Splitter utilities.",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
                0, 0, 0, 0, this->hwnd(), reinterpret_cast<HMENU>(102), GetModuleHandleW(nullptr), nullptr);

            // Initialize Splitter
            _splitter.create(this, 103, {0, 0}, {0, 0});
            _splitter.set_windows(_hEditTop, _hEditBottom);

            // Set GUI Font
            HFONT hFont = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
            SendMessage(_hEditTop, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), FALSE);
            SendMessage(_hEditBottom, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), FALSE);

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
