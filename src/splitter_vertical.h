#pragma once
#include <Windows.h>
#include "splitter_base.h"
#include "defer_window_pos.h"

/**
 * @brief Vertical splitter control dividing left and right panes.
 *
 * Implements a split bar that rests vertically between two child windows.
 * Dragging the bar horizontally updates the dimensions of both the left and
 * right panes. It uses IDC_SIZEWE (West-East) cursor.
 */
class splitter_vertical : public splitter_base<splitter_vertical> {
public:
    /**
     * @brief Creates and initializes the vertical splitter control.
     *
     * Registers the built-in control's class, sets up background painting logic,
     * and calls the base setup routine to bind the dragging handlers.
     */
    splitter_vertical() {
        this->setup.wndClassEx.lpszClassName = L"WINLAMB_SPLITTER_VERT";
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

    /**
     * @brief Returns the cursor resource ID for vertical splitters.
     *
     * Maps the cursor to IDC_SIZEWE since dragging the vertical
     * bar happens along the X axis.
     *
     * @return LPWSTR containing the IDC_SIZEWE system cursor identifier.
     */
    LPWSTR get_cursor() const noexcept {
        return IDC_SIZEWE;
    }

    /**
     * @brief Processes a mouse drag event.
     *
     * Computes the new width of the left window and offset of the right window
     * based on the X coordinate of the drag interaction. Evaluates parent boundaries
     * to avoid negative sizing. Finally, applies DeferWindowPos via RAII to ensure atomicity.
     *
     * @param x The current X local mouse position.
     * @param y The current Y local mouse position.
     */
    void on_drag(int x, int y) noexcept {
        (void)y; // Vertical bar moves horizontally only
        
        HWND hParent = GetParent(this->hwnd());
        if (!hParent) return;

        // Obtain mouse position in parent's client coordinates
        POINT pt{x, 0};
        ClientToScreen(this->hwnd(), &pt);
        ScreenToClient(hParent, &pt);

        RECT rcBar; GetWindowRect(this->hwnd(), &rcBar);
        RECT rc1;   GetWindowRect(_hwnd1, &rc1);
        RECT rc2;   GetWindowRect(_hwnd2, &rc2);

        MapWindowPoints(HWND_DESKTOP, hParent, reinterpret_cast<LPPOINT>(&rcBar), 2);
        MapWindowPoints(HWND_DESKTOP, hParent, reinterpret_cast<LPPOINT>(&rc1), 2);
        MapWindowPoints(HWND_DESKTOP, hParent, reinterpret_cast<LPPOINT>(&rc2), 2);

        int bar_thickness = rcBar.right - rcBar.left;
        
        // Enforce boundaries
        int min_x = rc1.left + 10;
        int max_x = rc2.right - bar_thickness - 10;
        
        int new_x = pt.x;
        if (new_x < min_x) new_x = min_x;
        if (new_x > max_x) new_x = max_x;

        int new_left_width = new_x - rc1.left;
        int new_right_left = new_x + bar_thickness;
        int new_right_width = rc2.right - new_right_left;

        defer_window_pos dwp(3);
        dwp.defer(_hwnd1, nullptr, rc1.left, rc1.top, new_left_width, rc1.bottom - rc1.top, SWP_NOZORDER | SWP_NOACTIVATE);
        dwp.defer(this->hwnd(), nullptr, new_x, rcBar.top, bar_thickness, rcBar.bottom - rcBar.top, SWP_NOZORDER | SWP_NOACTIVATE);
        dwp.defer(_hwnd2, nullptr, new_right_left, rc2.top, new_right_width, rc2.bottom - rc2.top, SWP_NOZORDER | SWP_NOACTIVATE);
    }
};
