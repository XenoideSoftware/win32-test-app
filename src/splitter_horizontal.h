#pragma once
#include <Windows.h>
#include "splitter_base.h"
#include "defer_window_pos.h"

/**
 * @brief Horizontal splitter control dividing top and bottom panes.
 *
 * It occupies the entire provided background area and lays out its two
 * child edits to the top and bottom, preserving the bar gap at `_split_pos`.
 */
class splitter_horizontal : public splitter_base<splitter_horizontal> {
public:
    splitter_horizontal() {
        this->setup.wndClassEx.lpszClassName = L"WINLAMB_SPLITTER_HORZ";
        this->setup.style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS;

        this->on_message(WM_PAINT, [this](wl::params p) -> LRESULT {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(this->hwnd(), &ps);
            RECT rc;
            GetClientRect(this->hwnd(), &rc);
            FillRect(hdc, &rc, reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1));
            EndPaint(this->hwnd(), &ps);
            return 0;
        });

        this->setup_handlers();
    }

    LPWSTR get_cursor() const noexcept {
        return IDC_SIZENS;
    }

    int get_primary_size(int w, int h) const noexcept {
        (void)w;
        return h;
    }

    void on_layout(int w, int h) noexcept {
        if (_split_pos == -1) {
            _split_pos = (h - _split_thickness) / 2;
        }

        if (_split_pos < 0) _split_pos = 0;
        if (_split_pos > h - _split_thickness) _split_pos = h - _split_thickness;
        if (_split_pos < 0) _split_pos = 0; // safe guard if h is very small

        defer_window_pos dwp(2);
        dwp.defer(_hwnd1, nullptr, 0, 0, w, _split_pos, SWP_NOZORDER | SWP_NOACTIVATE);
        dwp.defer(_hwnd2, nullptr, 0, _split_pos + _split_thickness, w, h - _split_pos - _split_thickness, SWP_NOZORDER | SWP_NOACTIVATE);
    }

    void on_drag(int x, int y) noexcept {
        (void)x; // Horizontal bar moves vertically only
        
        RECT rc;
        GetClientRect(this->hwnd(), &rc);
        int h = rc.bottom;
        
        // Ensure bounds
        int min_y = 10;
        int max_y = h - _split_thickness - 10;
        
        int new_y = y;
        if (new_y < min_y) new_y = min_y;
        if (new_y > max_y) new_y = max_y;

        _split_pos = new_y;
        on_layout(rc.right, rc.bottom);
    }
};
