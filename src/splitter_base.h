#pragma once
#include <Windows.h>
#include <WindowsX.h>
#include "winlamb/window_control.h"

/**
 * @brief Base CRTP class template for window splitter controls.
 *
 * Implements common splitter logic such as mouse capture, cursor handling,
 * and mouse movement tracking. Derived classes specialize the layout updates
 * and visual orientations for horizontal or vertical splitting.
 *
 * @tparam Derived The specialized splitter class.
 */
template <typename Derived>
class splitter_base : public wl::window_control {
protected:
    HWND _hwnd1 = nullptr;     ///< Handle to the first window (Left or Top).
    HWND _hwnd2 = nullptr;     ///< Handle to the second window (Right or Bottom).
    bool _is_dragging = false; ///< True if the user is currently capturing the mouse.

public:
    /**
     * @brief Assigns the two sibling windows to be resized.
     *
     * Caches the provided HWNDs internally. The derived implementations
     * will use these handles when their internal drag method invokes the layout recalculation.
     *
     * @param hwnd1 Handle to the first window (e.g., top or left).
     * @param hwnd2 Handle to the second window (e.g., bottom or right).
     */
    void set_windows(HWND hwnd1, HWND hwnd2) noexcept {
        _hwnd1 = hwnd1;
        _hwnd2 = hwnd2;
    }

protected:
    /**
     * @brief Initializes standard message handlers for drag operations.
     *
     * Registers handlers for WM_LBUTTONDOWN, WM_MOUSEMOVE, and WM_LBUTTONUP. 
     * Also requests the derived class to provide a specific system cursor via get_cursor().
     */
    void setup_handlers() {
        this->on_message(WM_SETCURSOR, [this](wl::params) -> LRESULT {
            SetCursor(LoadCursorW(nullptr, static_cast<Derived*>(this)->get_cursor()));
            return TRUE;
        });

        this->on_message(WM_LBUTTONDOWN, [this](wl::params) -> LRESULT {
            if (_hwnd1 && _hwnd2) {
                SetCapture(this->hwnd());
                _is_dragging = true;
            }
            return 0;
        });

        this->on_message(WM_MOUSEMOVE, [this](wl::params p) -> LRESULT {
            if (_is_dragging) {
                int x = GET_X_LPARAM(p.lParam);
                int y = GET_Y_LPARAM(p.lParam);
                static_cast<Derived*>(this)->on_drag(x, y);
            }
            return 0;
        });

        this->on_message(WM_LBUTTONUP, [this](wl::params) -> LRESULT {
            if (_is_dragging) {
                ReleaseCapture();
                _is_dragging = false;
            }
            return 0;
        });
    }
};
