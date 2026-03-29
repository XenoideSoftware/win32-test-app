#pragma once
#include <Windows.h>
#include "splitter_base.h"
#include "defer_window_pos.h"

/**
 * @brief Vertical splitter control dividing left and right panes.
 *
 * It occupies the entire provided background area and lays out its two
 * child edits to the left and right, preserving the bar gap at `_split_pos`.
 */
class splitter_vertical : public splitter_base<splitter_vertical> {
public:
    splitter_vertical() {
        this->setup.wndClassEx.lpszClassName = L"WINLAMB_SPLITTER_VERT";
        this->setup.style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS;

        this->on_message(WM_PAINT, [this](wl::params p) -> LRESULT {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(this->hwnd(), &ps);
            RECT rc;
            GetClientRect(this->hwnd(), &rc);
            // Paint only the background. The child edit windows will cover the rest.
            FillRect(hdc, &rc, reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1));
            EndPaint(this->hwnd(), &ps);
            return 0;
        });

        this->setup_handlers();
    }

    LPTSTR get_cursor() const noexcept {
        return IDC_SIZEWE;
    }

    int get_primary_size(int w, int h) const noexcept {
        (void)h;
        return w;
    }

    void on_layout(int w, int h) noexcept {
        if (_split_pos == -1) {
            _split_pos = (w - _split_thickness) / 2;
        }

        if (_split_pos < 0) _split_pos = 0;
        if (_split_pos > w - _split_thickness) _split_pos = w - _split_thickness;
        if (_split_pos < 0) _split_pos = 0; // safe guard if w is very small

        defer_window_pos dwp(2);
        dwp.defer(_hwnd1, nullptr, 0, 0, _split_pos, h, SWP_NOZORDER | SWP_NOACTIVATE);
        dwp.defer(_hwnd2, nullptr, _split_pos + _split_thickness, 0, w - _split_pos - _split_thickness, h, SWP_NOZORDER | SWP_NOACTIVATE);
    }

    void on_drag(int x, int y) noexcept {
        (void)y; // Vertical bar moves horizontally only
        
        RECT rc;
        GetClientRect(this->hwnd(), &rc);
        int w = rc.right;
        
        // Ensure bounds
        int min_x = 10;
        int max_x = w - _split_thickness - 10;
        
        int new_x = x;
        if (new_x < min_x) new_x = min_x;
        if (new_x > max_x) new_x = max_x;

        _split_pos = new_x;
        on_layout(rc.right, rc.bottom);
    }
};
