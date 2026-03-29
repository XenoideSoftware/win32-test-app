#pragma once
#include <Windows.h>
#include "splitter_base.h"
#include "defer_window_pos.h"

/**
 * @brief Horizontal splitter control dividing top and bottom panes.
 *
 * Implements a split bar that rests horizontally between two child windows.
 * Dragging the bar vertically updates the dimensions of both the top and
 * bottom panes. It uses IDC_SIZENS (North-South) cursor.
 */
class splitter_horizontal : public splitter_base<splitter_horizontal> {
public:
    /**
     * @brief Creates and initializes the horizontal splitter control.
     *
     * Registers the built-in control's class, sets up background painting logic,
     * and calls the base setup routine to bind the dragging handlers.
     */
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

    /**
     * @brief Returns the cursor resource ID for horizontal splitters.
     *
     * Maps the cursor to IDC_SIZENS since dragging the horizontal
     * bar happens along the Y axis.
     *
     * @return LPWSTR containing the IDC_SIZENS system cursor identifier.
     */
    LPWSTR get_cursor() const noexcept {
        return IDC_SIZENS;
    }

    /**
     * @brief Processes a mouse drag event.
     *
     * Computes the new height of the top window and offset of the bottom window
     * based on the Y coordinate of the drag interaction. Evaluates parent boundaries
     * to avoid negative sizing. Finally, applies DeferWindowPos via RAII to ensure atomicity.
     *
     * @param x The current X local mouse position.
     * @param y The current Y local mouse position.
     */
    void on_drag(int x, int y) noexcept {
        (void)x; // Horizontal bar moves vertically only
        
        HWND hParent = GetParent(this->hwnd());
        if (!hParent) return;

        // Obtain mouse position in parent's client coordinates
        POINT pt{0, y};
        ClientToScreen(this->hwnd(), &pt);
        ScreenToClient(hParent, &pt);

        RECT rcBar; GetWindowRect(this->hwnd(), &rcBar);
        RECT rc1;   GetWindowRect(_hwnd1, &rc1);
        RECT rc2;   GetWindowRect(_hwnd2, &rc2);

        MapWindowPoints(HWND_DESKTOP, hParent, reinterpret_cast<LPPOINT>(&rcBar), 2);
        MapWindowPoints(HWND_DESKTOP, hParent, reinterpret_cast<LPPOINT>(&rc1), 2);
        MapWindowPoints(HWND_DESKTOP, hParent, reinterpret_cast<LPPOINT>(&rc2), 2);

        int bar_thickness = rcBar.bottom - rcBar.top;
        
        // Enforce boundaries
        int min_y = rc1.top + 10;
        int max_y = rc2.bottom - bar_thickness - 10;
        
        int new_y = pt.y;
        if (new_y < min_y) new_y = min_y;
        if (new_y > max_y) new_y = max_y;

        int new_top_height = new_y - rc1.top;
        int new_bottom_top = new_y + bar_thickness;
        int new_bottom_height = rc2.bottom - new_bottom_top;

        defer_window_pos dwp(3);
        dwp.defer(_hwnd1, nullptr, rc1.left, rc1.top, rc1.right - rc1.left, new_top_height, SWP_NOZORDER | SWP_NOACTIVATE);
        dwp.defer(this->hwnd(), nullptr, rcBar.left, new_y, rcBar.right - rcBar.left, bar_thickness, SWP_NOZORDER | SWP_NOACTIVATE);
        dwp.defer(_hwnd2, nullptr, rc2.left, new_bottom_top, rc2.right - rc2.left, new_bottom_height, SWP_NOZORDER | SWP_NOACTIVATE);
    }
};
