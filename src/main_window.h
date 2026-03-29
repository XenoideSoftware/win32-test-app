/**
 * @file main_window.h
 * @brief Main application window: menu bar, status bar, left panel, tabbed centre area.
 *
 * Part of ModernWinApp.
 */

#pragma once
#include <Windows.h>
#include <CommCtrl.h>
#include "winlamb/window_main.h"
#include "winlamb/statusbar.h"
#include "wrappers/tabctrl.h"
#include "left_panel.h"
#include "tab_page_richtext.h"
#include "tab_page_webview.h"

/**
 * @brief Root application window.
 *
 * Layout:
 * @verbatim
 * ┌─────────────────────────────────────────────────┐
 * │  [File] [Edit] [View] [Help]    ← Win32 menu   │
 * ├───────────┬─────────────────────────────────────┤
 * │           │  [Rich Text ×] [Web View ×]         │
 * │ TreeView  │  ┌───────────────────────────────┐  │
 * │ (220 px)  │  │  Tab page content              │  │
 * │           │  └───────────────────────────────┘  │
 * ├───────────┴─────────────────────────────────────┤
 * │  [Ready]                    [ModernWinApp v1.0] │
 * └─────────────────────────────────────────────────┘
 * @endverbatim
 */
class MainWindow : public wl::window_main {
public:
    /// @brief Control and menu command identifiers.
    enum CtrlId : int {
        IDC_LEFT_PANEL    = 100, ///< Left panel (LeftPanel window_control)
        IDC_TREEVIEW      = 101, ///< Treeview inside the left panel
        IDC_TABCTRL       = 200, ///< Centre tab control
        IDC_TAB_RICHTEXT  = 201, ///< Rich-text tab page
        IDC_TAB_WEBVIEW   = 202, ///< Web-view tab page

        IDM_FILE_NEW   = 1001,
        IDM_FILE_OPEN  = 1002,
        IDM_FILE_SAVE  = 1003,
        IDM_FILE_EXIT  = 1004,
        IDM_EDIT_CUT   = 1010,
        IDM_EDIT_COPY  = 1011,
        IDM_EDIT_PASTE = 1012,
        IDM_VIEW_PANEL = 1020,
        IDM_HELP_ABOUT = 1030,
    };

private:
    wl::statusbar   _statusbar;
    wl::tabctrl     _tabctrl;
    LeftPanel       _leftPanel;
    TabPageRichText _richTextPage;
    TabPageWebView  _webViewPage;
    bool            _leftPanelVisible = true;

    static constexpr int LEFT_PANEL_WIDTH = 220; ///< Fixed left panel width in pixels.

    // -------------------------------------------------------------------------

    /**
     * @brief Builds the application menu bar and attaches it to the window.
     */
    void _create_menu() noexcept {
        HMENU hBar = CreateMenu();

        HMENU hFile = CreatePopupMenu();
        AppendMenuW(hFile, MF_STRING,    IDM_FILE_NEW,  L"&New\tCtrl+N");
        AppendMenuW(hFile, MF_STRING,    IDM_FILE_OPEN, L"&Open...\tCtrl+O");
        AppendMenuW(hFile, MF_STRING,    IDM_FILE_SAVE, L"&Save\tCtrl+S");
        AppendMenuW(hFile, MF_SEPARATOR, 0,             nullptr);
        AppendMenuW(hFile, MF_STRING,    IDM_FILE_EXIT, L"E&xit\tAlt+F4");
        AppendMenuW(hBar,  MF_POPUP, reinterpret_cast<UINT_PTR>(hFile), L"&File");

        HMENU hEdit = CreatePopupMenu();
        AppendMenuW(hEdit, MF_STRING, IDM_EDIT_CUT,   L"Cu&t\tCtrl+X");
        AppendMenuW(hEdit, MF_STRING, IDM_EDIT_COPY,  L"&Copy\tCtrl+C");
        AppendMenuW(hEdit, MF_STRING, IDM_EDIT_PASTE, L"&Paste\tCtrl+V");
        AppendMenuW(hBar,  MF_POPUP, reinterpret_cast<UINT_PTR>(hEdit), L"&Edit");

        HMENU hView = CreatePopupMenu();
        AppendMenuW(hView, MF_STRING, IDM_VIEW_PANEL, L"Toggle &Panel\tCtrl+B");
        AppendMenuW(hBar,  MF_POPUP, reinterpret_cast<UINT_PTR>(hView), L"&View");

        HMENU hHelp = CreatePopupMenu();
        AppendMenuW(hHelp, MF_STRING, IDM_HELP_ABOUT, L"&About...");
        AppendMenuW(hBar,  MF_POPUP, reinterpret_cast<UINT_PTR>(hHelp), L"&Help");

        SetMenu(this->hwnd(), hBar);
    }

    /**
     * @brief Returns the height of the status bar in pixels (0 if not yet created).
     */
    int _statusbar_height() const noexcept {
        if (!this->_statusbar.hwnd()) return 0;
        RECT rc{};
        GetWindowRect(this->_statusbar.hwnd(), &rc);
        return rc.bottom - rc.top;
    }

    /**
     * @brief Repositions all child windows to match the current client area.
     *
     * Uses BeginDeferWindowPos for flicker-free simultaneous repositioning of the
     * left panel and tab control. Tab page windows are repositioned afterwards
     * based on the tab control's adjusted display area.
     */
    void _layout_children() noexcept {
        if (!this->hwnd() || !this->_tabctrl.hwnd()) return;

        RECT clientRc{};
        GetClientRect(this->hwnd(), &clientRc);
        const int sbH    = _statusbar_height();
        const int clientW = clientRc.right;
        const int clientH = clientRc.bottom - sbH;
        const int panelW  = _leftPanelVisible ? LEFT_PANEL_WIDTH : 0;
        const int tabW    = clientW - panelW;

        // --- Left panel + tab control (atomic resize) ---
        HDWP hdwp = BeginDeferWindowPos(2);
        if (this->_leftPanel.hwnd()) {
            hdwp = DeferWindowPos(hdwp, this->_leftPanel.hwnd(), nullptr,
                0, 0, panelW, clientH,
                SWP_NOZORDER | SWP_NOACTIVATE);
        }
        if (this->_tabctrl.hwnd()) {
            hdwp = DeferWindowPos(hdwp, this->_tabctrl.hwnd(), nullptr,
                panelW, 0, tabW, clientH,
                SWP_NOZORDER | SWP_NOACTIVATE);
        }
        EndDeferWindowPos(hdwp);
        ShowWindow(this->_leftPanel.hwnd(),
            _leftPanelVisible ? SW_SHOW : SW_HIDE);

        // --- Tab page content area ---
        // Compute display area for the NEW tab control dimensions.
        RECT pageRc{ 0, 0, tabW, clientH };
        TabCtrl_AdjustRect(this->_tabctrl.hwnd(), FALSE, &pageRc);
        // Shift from tab-control client coords to main-window client coords.
        OffsetRect(&pageRc, panelW, 0);
        const LONG pageCx = pageRc.right  - pageRc.left;
        const LONG pageCy = pageRc.bottom - pageRc.top;

        if (this->_richTextPage.hwnd()) {
            SetWindowPos(this->_richTextPage.hwnd(), nullptr,
                pageRc.left, pageRc.top, pageCx, pageCy,
                SWP_NOZORDER | SWP_NOACTIVATE);
        }
        if (this->_webViewPage.hwnd()) {
            SetWindowPos(this->_webViewPage.hwnd(), nullptr,
                pageRc.left, pageRc.top, pageCx, pageCy,
                SWP_NOZORDER | SWP_NOACTIVATE);
        }
    }

    /**
     * @brief Shows the tab page that corresponds to the currently selected tab.
     *
     * Tab 0 → Rich Text page, Tab 1 → Web View page.
     */
    void _sync_tab_pages() noexcept {
        const int sel = this->_tabctrl.selected_index();
        ShowWindow(this->_richTextPage.hwnd(), sel == 0 ? SW_SHOW : SW_HIDE);
        ShowWindow(this->_webViewPage.hwnd(),  sel == 1 ? SW_SHOW : SW_HIDE);
    }

public:
    /**
     * @brief Constructs the main window and registers all message handlers.
     */
    MainWindow() {
        this->setup.wndClassEx.lpszClassName = L"MODERNWINAPP_MAIN";
        this->setup.title = L"ModernWinApp";
        this->setup.style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
        this->setup.size  = {1200, 700};

        // ---- WM_CREATE --------------------------------------------------
        this->on_message(WM_CREATE, [this](wl::params) -> LRESULT {
            _create_menu(); // Must precede GetClientRect (affects client area).

            // Status bar
            this->_statusbar.create(this);
            this->_statusbar.add_resizable_part(3);
            this->_statusbar.add_fixed_part(160);
            this->_statusbar.set_text(L"Ready", 0);
            this->_statusbar.set_text(L"ModernWinApp v1.0", 1);

            RECT rc{};
            GetClientRect(this->hwnd(), &rc);
            const int sbH  = _statusbar_height();
            const int w    = rc.right;
            const int h    = rc.bottom - sbH;

            // Left panel
            this->_leftPanel.create(this, IDC_LEFT_PANEL,
                {0, 0}, {LEFT_PANEL_WIDTH, h});

            // Tab control
            this->_tabctrl.create(this, IDC_TABCTRL,
                {LEFT_PANEL_WIDTH, 0}, {w - LEFT_PANEL_WIDTH, h});
            this->_tabctrl.add_tab(L"Rich Text");
            this->_tabctrl.add_tab(L"Web View");
            this->_tabctrl.on_close_tab([this](size_t idx) {
                this->_tabctrl.remove_tab(idx);
                _sync_tab_pages();
                _layout_children();
            });

            // Compute initial tab page area
            RECT pageRc{ 0, 0, w - LEFT_PANEL_WIDTH, h };
            TabCtrl_AdjustRect(this->_tabctrl.hwnd(), FALSE, &pageRc);
            OffsetRect(&pageRc, LEFT_PANEL_WIDTH, 0);
            const SIZE pageSize{
                pageRc.right  - pageRc.left,
                pageRc.bottom - pageRc.top
            };

            // Tab pages (children of main window, drawn over tab display area)
            this->_richTextPage.create(this, IDC_TAB_RICHTEXT,
                {pageRc.left, pageRc.top}, pageSize);
            this->_webViewPage.create(this, IDC_TAB_WEBVIEW,
                {pageRc.left, pageRc.top}, pageSize);
            ShowWindow(this->_webViewPage.hwnd(), SW_HIDE);

            return 0;
        });

        // ---- WM_SIZE ----------------------------------------------------
        this->on_message(WM_SIZE, [this](wl::params p) -> LRESULT {
            if (p.wParam == SIZE_MINIMIZED) return 0;
            this->_statusbar.adjust(p);
            _layout_children();
            return 0;
        });

        // ---- WM_DRAWITEM (owner-draw tab control) -------------------------
        this->on_message(WM_DRAWITEM, [this](wl::params p) -> LRESULT {
            const DRAWITEMSTRUCT& dis =
                *reinterpret_cast<DRAWITEMSTRUCT*>(p.lParam);
            if (static_cast<int>(dis.CtlID) == IDC_TABCTRL) {
                this->_tabctrl.draw_item(dis);
                return TRUE;
            }
            return 0;
        });

        // ---- WM_NOTIFY (tab selection change) ----------------------------
        this->on_notify(IDC_TABCTRL, TCN_SELCHANGE,
            [this](wl::params) -> LRESULT
        {
            _sync_tab_pages();
            return 0;
        });

        // ---- WM_COMMAND (menu items) -------------------------------------
        this->on_command(static_cast<WORD>(IDM_FILE_EXIT),
            [this](wl::params) -> LRESULT
        {
            PostMessageW(this->hwnd(), WM_CLOSE, 0, 0);
            return 0;
        });

        this->on_command(static_cast<WORD>(IDM_VIEW_PANEL),
            [this](wl::params) -> LRESULT
        {
            _leftPanelVisible = !_leftPanelVisible;
            _layout_children();
            return 0;
        });

        this->on_command(static_cast<WORD>(IDM_HELP_ABOUT),
            [this](wl::params) -> LRESULT
        {
            MessageBoxW(this->hwnd(),
                L"ModernWinApp v1.0\n\n"
                L"A mock Windows native application\n"
                L"built with WinLamb.",
                L"About ModernWinApp",
                MB_OK | MB_ICONINFORMATION);
            return 0;
        });

        this->on_command(static_cast<WORD>(IDM_FILE_NEW),
            [this](wl::params) -> LRESULT
        {
            this->_statusbar.set_text(L"New file…", 0);
            return 0;
        });
    }

    MainWindow(MainWindow&&) = default;
    MainWindow& operator=(MainWindow&&) = default; ///< Move-only.
};
