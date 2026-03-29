/**
 * @file tabctrl.h
 * @brief WinLamb wrapper for the Win32 tab control (WC_TABCONTROL).
 *
 * Part of ModernWinApp — follows the WinLamb header-only, move-only pattern.
 */

#pragma once
#include <functional>
#include <string>
#include <vector>
#include <stdexcept>
#include <Windows.h>
#include <CommCtrl.h>
#include <windowsx.h>
#include "winlamb/internals/base_native_ctrl.h"
#include "winlamb/internals/base_native_ctrl_pubm.h"
#include "winlamb/internals/base_focus_pubm.h"
#include "winlamb/internals/styler.h"
#include "winlamb/subclass.h"
#include "winlamb/wnd.h"

namespace wl {

/**
 * @brief WinLamb wrapper for the Win32 tab control (WC_TABCONTROL).
 *
 * Supports owner-drawn tabs with per-tab close buttons. The owning window
 * **must** forward its `WM_DRAWITEM` message to draw_item() for tabs to render.
 *
 * @par Minimal usage
 * @code
 * // Member:
 * wl::tabctrl _tabs;
 *
 * // WM_CREATE:
 * _tabs.create(this, IDC_TABS, {0, 0}, {800, 600});
 * _tabs.add_tab(L"Tab 1").add_tab(L"Tab 2");
 * _tabs.on_close_tab([this](size_t i){ _tabs.remove_tab(i); });
 *
 * // WM_DRAWITEM:
 * const auto& dis = *reinterpret_cast<DRAWITEMSTRUCT*>(p.lParam);
 * if (dis.CtlID == IDC_TABS) { _tabs.draw_item(dis); return TRUE; }
 * @endcode
 */
class tabctrl final :
    public wnd,
    public _wli::base_native_ctrl_pubm<tabctrl>,
    public _wli::base_focus_pubm<tabctrl>
{
private:
    HWND                        _hWnd = nullptr;
    _wli::base_native_ctrl      _baseNativeCtrl{_hWnd};
    wl::subclass                _subclass;
    std::vector<std::wstring>   _labels;
    std::function<void(size_t)> _onCloseTab;

    static constexpr int CLOSE_BTN_SIZE   = 14; ///< Close button side length in pixels.
    static constexpr int CLOSE_BTN_MARGIN = 3;  ///< Gap between close button and tab right edge.

    /**
     * @brief Returns the close-button bounding rect for the given tab rect.
     * @param tabRc  Tab bounding rect (from TabCtrl_GetItemRect).
     */
    static RECT _close_btn_rect(const RECT& tabRc) noexcept {
        int cy       = tabRc.bottom - tabRc.top;
        int btnTop   = tabRc.top    + (cy - CLOSE_BTN_SIZE) / 2;
        int btnRight = tabRc.right  - CLOSE_BTN_MARGIN;
        return { btnRight - CLOSE_BTN_SIZE, btnTop,
                 btnRight,                  btnTop + CLOSE_BTN_SIZE };
    }

    /**
     * @brief Hit-tests @p pt against every tab's close button.
     * @return Zero-based tab index, or -1 if no close button was hit.
     */
    int _hit_test_close(POINT pt) const noexcept {
        int n = TabCtrl_GetItemCount(this->_hWnd);
        for (int i = 0; i < n; ++i) {
            RECT tabRc{};
            TabCtrl_GetItemRect(this->_hWnd, i, &tabRc);
            const RECT closeRc = _close_btn_rect(tabRc);
            if (PtInRect(&closeRc, pt)) return i;
        }
        return -1;
    }

public:
    /** @brief Wraps window style changes via Get/SetWindowLongPtr. */
    _wli::styler<tabctrl> style{this};

    /**
     * @brief Constructs the tabctrl and registers the subclass close-button handler.
     *
     * The WM_LBUTTONDOWN handler is registered here (before any HWND exists) so
     * that it is in place before the first message arrives after install_subclass().
     */
    tabctrl() :
        wnd(_hWnd),
        base_native_ctrl_pubm(_baseNativeCtrl),
        base_focus_pubm(_hWnd)
    {
        // Register subclass handler before install so _canAdd is still true.
        this->_subclass.on_message(WM_LBUTTONDOWN, [this](wl::params p) -> LRESULT {
            POINT pt{ GET_X_LPARAM(p.lParam), GET_Y_LPARAM(p.lParam) };
            const int idx = this->_hit_test_close(pt);
            if (idx >= 0 && this->_onCloseTab) {
                this->_onCloseTab(static_cast<size_t>(idx));
                return 0;
            }
            // Not a close-button hit — delegate to the tab control's own WndProc.
            return DefSubclassProc(this->_hWnd, p.message, p.wParam, p.lParam);
        });
    }

    tabctrl(tabctrl&&) = default;
    tabctrl& operator=(tabctrl&&) = default; ///< Move-only.

    /**
     * @brief Creates the tab control as a child window.
     * @param hParent  Parent window handle.
     * @param ctrlId   Child-window control identifier.
     * @param pos      Top-left position in parent client coordinates.
     * @param size     Width and height of the control.
     */
    tabctrl& create(HWND hParent, int ctrlId, POINT pos, SIZE size) {
        this->_baseNativeCtrl.create(hParent, ctrlId, nullptr, pos, size,
            WC_TABCONTROLW,
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TCS_OWNERDRAWFIXED,
            0);
        // Subclass to intercept WM_LBUTTONDOWN for close-button hit testing.
        this->_subclass.install_subclass(this->_hWnd);
        return *this;
    }

    /** @overload Creates as a child of a wnd-derived parent. */
    tabctrl& create(const wnd* parent, int ctrlId, POINT pos, SIZE size) {
        return this->create(parent->hwnd(), ctrlId, pos, size);
    }

    /**
     * @brief Appends a new tab with the given label.
     * @param label  Tab caption (null-terminated wide string).
     */
    tabctrl& add_tab(const wchar_t* label) {
        this->_labels.emplace_back(label);
        TCITEMW tci{};
        tci.mask    = TCIF_TEXT;
        tci.pszText = const_cast<wchar_t*>(label);
        TabCtrl_InsertItem(this->_hWnd,
            static_cast<int>(this->_labels.size() - 1), &tci);
        return *this;
    }

    /** @overload */
    tabctrl& add_tab(const std::wstring& label) {
        return this->add_tab(label.c_str());
    }

    /**
     * @brief Removes the tab at @p index.
     * @param index  Zero-based index of the tab to remove.
     */
    tabctrl& remove_tab(size_t index) {
        if (index < this->_labels.size()) {
            TabCtrl_DeleteItem(this->_hWnd, static_cast<int>(index));
            this->_labels.erase(
                this->_labels.begin() + static_cast<ptrdiff_t>(index));
        }
        return *this;
    }

    /**
     * @brief Returns the zero-based index of the currently selected tab, or -1.
     */
    int selected_index() const noexcept {
        return TabCtrl_GetCurSel(this->_hWnd);
    }

    /**
     * @brief Programmatically selects the tab at @p index.
     * @param index  Zero-based tab index.
     */
    tabctrl& select(size_t index) noexcept {
        TabCtrl_SetCurSel(this->_hWnd, static_cast<int>(index));
        return *this;
    }

    /**
     * @brief Returns the total number of tabs.
     */
    size_t count() const noexcept {
        return static_cast<size_t>(TabCtrl_GetItemCount(this->_hWnd));
    }

    /**
     * @brief Returns the tab content display area in the control's client coordinates.
     *
     * Tab page child windows should be mapped from this coordinate system to the
     * parent window's client coordinates using MapWindowPoints before positioning.
     *
     * @code
     * RECT area = _tabctrl.display_area();
     * MapWindowPoints(_tabctrl.hwnd(), hwnd(), (LPPOINT)&area, 2);
     * // area is now in main-window client coordinates
     * @endcode
     */
    RECT display_area() const noexcept {
        RECT rc{};
        GetClientRect(this->_hWnd, &rc);
        TabCtrl_AdjustRect(this->_hWnd, FALSE, &rc);
        return rc;
    }

    /**
     * @brief Registers a callback invoked when the user clicks a tab's close button.
     * @param fn  Callable receiving the zero-based index of the tab to close.
     *            Typically calls remove_tab() and updates page visibility.
     */
    tabctrl& on_close_tab(std::function<void(size_t)> fn) {
        this->_onCloseTab = std::move(fn);
        return *this;
    }

    /**
     * @brief Draws a single owner-draw tab item.
     *
     * Must be called from the **parent window's** WM_DRAWITEM handler.
     * The parent receives WM_DRAWITEM because TCS_OWNERDRAWFIXED is set.
     *
     * @param dis  DRAWITEMSTRUCT delivered by WM_DRAWITEM.
     */
    void draw_item(const DRAWITEMSTRUCT& dis) noexcept {
        HDC  hdc = dis.hDC;
        RECT rc  = dis.rcItem;
        bool sel = (dis.itemState & ODS_SELECTED) != 0;

        // Background
        HBRUSH hBg = CreateSolidBrush(
            GetSysColor(sel ? COLOR_BTNFACE : COLOR_BTNHIGHLIGHT));
        FillRect(hdc, &rc, hBg);
        DeleteObject(hBg);

        // Label text — reserve right margin for the close button
        RECT textRc   = rc;
        textRc.left  += 6;
        textRc.right  = rc.right - CLOSE_BTN_SIZE - CLOSE_BTN_MARGIN * 2;

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));

        const std::wstring& label = (dis.itemID < this->_labels.size())
            ? this->_labels[dis.itemID]
            : std::wstring{};
        DrawTextW(hdc, label.c_str(), -1, &textRc,
            DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

        // Close button "×"
        RECT closeRc = _close_btn_rect(rc);
        SetTextColor(hdc, RGB(90, 90, 90));
        DrawTextW(hdc, L"\u00D7", -1, &closeRc,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        // Focus rectangle
        if (dis.itemState & ODS_FOCUS) {
            DrawFocusRect(hdc, &rc);
        }
    }
};

} // namespace wl
