#pragma once
#include <Windows.h>
#include <WindowsX.h>
#include "winlamb/window_control.h"

/**
 * @brief Base CRTP class template for window splitter controls.
 *
 * Implements common splitter logic such as mouse capture, cursor handling,
 * and mouse movement tracking. The splitter window itself occupies the
 * entire client area allocated to it by its parent, and it arranges its
 * two assigned child windows over itself, leaving a gap for dragging.
 *
 * @tparam Derived The specialized splitter class.
 */
template <typename Derived>
class splitter_base : public wl::window_control {
protected:
    HWND _hwnd1 = nullptr;     ///< Handle to the first window (Left or Top).
    HWND _hwnd2 = nullptr;     ///< Handle to the second window (Right or Bottom).
    bool _is_dragging = false; ///< True if the user is currently capturing the mouse.

    int _split_pos = 200;      ///< Current coordinate position of the splitter.
    int _split_thickness = 6;  ///< Thickness of the splitter bar in pixels.

public:
    /**
     * @brief Assigns the two sibling windows to be resized.
     *
     * @param hwnd1 Handle to the first window (e.g., top or left).
     * @param hwnd2 Handle to the second window (e.g., bottom or right).
     */
    void set_windows(HWND hwnd1, HWND hwnd2) noexcept {
        _hwnd1 = hwnd1;
        _hwnd2 = hwnd2;
    }

    /**
     * @brief Manually sets the position of the splitter.
     * @param pos The new coordinate coordinate.
     */
    void set_split_pos(int pos) noexcept {
        _split_pos = pos;
        if (this->hwnd()) {
            RECT rc;
            GetClientRect(this->hwnd(), &rc);
            static_cast<Derived*>(this)->on_layout(rc.right, rc.bottom);
        }
    }

protected:
    /**
     * @brief Initializes standard message handlers for drag operations.
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

        this->on_message(WM_SIZE, [this](wl::params p) -> LRESULT {
            if (p.wParam != SIZE_MINIMIZED && _hwnd1 && _hwnd2) {
                int w = LOWORD(p.lParam);
                int h = HIWORD(p.lParam);
                static_cast<Derived*>(this)->on_layout(w, h);
            }
            return 0;
        });
    }
};
