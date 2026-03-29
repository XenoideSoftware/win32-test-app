/**
 * @file left_panel.h
 * @brief Left-side explorer panel hosting a sample treeview.
 *
 * Part of ModernWinApp.
 */

#pragma once
#include <Windows.h>
#include "winlamb/window_control.h"
#include "winlamb/treeview.h"

/**
 * @brief Resizable left panel containing a sample project treeview.
 *
 * Inherits wl::window_control so it can be embedded as a child window
 * inside the main application window and receives its own message pump.
 */
class LeftPanel : public wl::window_control {
private:
    wl::treeview _treeview;

    static constexpr int IDC_TREEVIEW_INNER = 101;

public:
    /**
     * @brief Constructs the panel and registers WM_CREATE / WM_SIZE handlers.
     *
     * The window class name `WL_LEFTPANEL` is assigned here, before create()
     * is called by the parent.
     */
    LeftPanel() {
        this->setup.wndClassEx.lpszClassName = L"WL_LEFTPANEL";
        this->setup.wndClassEx.hbrBackground =
            reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);

        this->on_message(WM_CREATE, [this](wl::params) -> LRESULT {
            RECT rc{};
            GetClientRect(this->hwnd(), &rc);
            this->_treeview.create(this->hwnd(), IDC_TREEVIEW_INNER,
                {0, 0}, {rc.right, rc.bottom});

            // --- Sample project tree ---
            auto project = this->_treeview.items.add_root(L"Project");
            project.add_child(L"src");
            project.add_child(L"include");
            project.add_child(L"docs");

            auto deps = this->_treeview.items.add_root(L"Dependencies");
            deps.add_child(L"winlamb");
            deps.add_child(L"mctrl");
            deps.add_child(L"HexCtrl");
            return 0;
        });

        this->on_message(WM_SIZE, [this](wl::params p) -> LRESULT {
            const int cx = LOWORD(p.lParam);
            const int cy = HIWORD(p.lParam);
            if (this->_treeview.hwnd()) {
                SetWindowPos(this->_treeview.hwnd(), nullptr,
                    0, 0, cx, cy,
                    SWP_NOZORDER | SWP_NOACTIVATE);
            }
            return 0;
        });
    }

    LeftPanel(LeftPanel&&) = default;
    LeftPanel& operator=(LeftPanel&&) = default; ///< Move-only.
};
