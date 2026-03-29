/**
 * @file richedit.h
 * @brief WinLamb wrapper for the modern RichEdit control (MSFTEDIT_CLASS / RICHEDIT50W).
 *
 * Part of ModernWinApp — follows the WinLamb header-only, move-only pattern.
 */

#pragma once
#include <string>
#include <stdexcept>
#include <Windows.h>
#include <Richedit.h>
#include "winlamb/internals/base_native_ctrl.h"
#include "winlamb/internals/base_native_ctrl_pubm.h"
#include "winlamb/internals/base_focus_pubm.h"
#include "winlamb/internals/styler.h"
#include "winlamb/wnd.h"

namespace wl {

/**
 * @brief WinLamb wrapper for the modern RichEdit control (RICHEDIT50W).
 *
 * Loads `Msftedit.dll` (with `riched20.dll` as fallback) the first time a
 * richedit is created; subsequent calls reuse the already-loaded module.
 *
 * The control is created with `ES_MULTILINE`, `WS_VSCROLL`, `ES_AUTOVSCROLL`,
 * `ES_WANTRETURN`, and `WS_EX_CLIENTEDGE` by default.
 */
class richedit final :
    public wnd,
    public _wli::base_native_ctrl_pubm<richedit>,
    public _wli::base_focus_pubm<richedit>
{
private:
    HWND                   _hWnd = nullptr;
    _wli::base_native_ctrl _baseNativeCtrl{_hWnd};

    /**
     * @brief Loads the RichEdit DLL exactly once per process.
     * @throws std::runtime_error if neither Msftedit.dll nor riched20.dll can be loaded.
     */
    static HMODULE _ensure_loaded() {
        static HMODULE h = []() -> HMODULE {
            HMODULE mod = LoadLibraryW(L"Msftedit.dll");
            if (!mod) mod = LoadLibraryW(L"riched20.dll");
            return mod;
        }();
        if (!h) throw std::runtime_error("Failed to load RichEdit DLL");
        return h;
    }

public:
    /** @brief Wraps window style changes via Get/SetWindowLongPtr. */
    _wli::styler<richedit> style{this};

    richedit() :
        wnd(_hWnd),
        base_native_ctrl_pubm(_baseNativeCtrl),
        base_focus_pubm(_hWnd) { }

    richedit(richedit&&) = default;
    richedit& operator=(richedit&&) = default; ///< Move-only.

    /**
     * @brief Creates the RichEdit control as a child window.
     *
     * Loads `Msftedit.dll` before the CreateWindowEx call.
     *
     * @param hParent  Parent window handle.
     * @param ctrlId   Child-window control identifier.
     * @param pos      Top-left position in parent client coordinates.
     * @param size     Width and height of the control.
     */
    richedit& create(HWND hParent, int ctrlId, POINT pos, SIZE size) {
        _ensure_loaded();
        this->_baseNativeCtrl.create(hParent, ctrlId, nullptr, pos, size,
            MSFTEDIT_CLASS,
            WS_CHILD | WS_VISIBLE | ES_MULTILINE |
                WS_VSCROLL | ES_AUTOVSCROLL | ES_WANTRETURN,
            WS_EX_CLIENTEDGE);
        return *this;
    }

    /** @overload Creates as a child of a wnd-derived parent. */
    richedit& create(const wnd* parent, int ctrlId, POINT pos, SIZE size) {
        return this->create(parent->hwnd(), ctrlId, pos, size);
    }

    /**
     * @brief Replaces all text in the control.
     * @param text  New content (wide string).
     */
    richedit& set_text(const std::wstring& text) noexcept {
        SetWindowTextW(this->_hWnd, text.c_str());
        return *this;
    }

    /**
     * @brief Returns the full text content of the control.
     */
    std::wstring get_text() const {
        const int len = GetWindowTextLengthW(this->_hWnd);
        if (!len) return {};
        std::wstring buf(static_cast<size_t>(len) + 1, L'\0');
        GetWindowTextW(this->_hWnd, buf.data(), len + 1);
        buf.resize(static_cast<size_t>(len));
        return buf;
    }

    /**
     * @brief Appends @p text at the end of the current content.
     *
     * Uses EM_SETSEL(-1, -1) + EM_REPLACESEL to avoid clearing the undo buffer.
     *
     * @param text  Wide string to append.
     */
    richedit& append_text(const std::wstring& text) noexcept {
        SendMessageW(this->_hWnd, EM_SETSEL,
            static_cast<WPARAM>(-1), static_cast<LPARAM>(-1));
        SendMessageW(this->_hWnd, EM_REPLACESEL,
            FALSE, reinterpret_cast<LPARAM>(text.c_str()));
        return *this;
    }

    /**
     * @brief Applies a font to all text in the control via CHARFORMAT2.
     *
     * Sets CFM_FACE and CFM_SIZE with SCF_ALL scope.
     *
     * @param face    Font face name (e.g. `L"Consolas"`).
     * @param sizePt  Point size (e.g. `11`). Internally stored as half-points.
     */
    richedit& set_font(const wchar_t* face, int sizePt) noexcept {
        CHARFORMAT2W cf{};
        cf.cbSize  = sizeof(cf);
        cf.dwMask  = CFM_FACE | CFM_SIZE;
        cf.yHeight = sizePt * 20; // half-points
        wcsncpy_s(cf.szFaceName, face, LF_FACESIZE - 1);
        SendMessageW(this->_hWnd, EM_SETCHARFORMAT,
            SCF_ALL, reinterpret_cast<LPARAM>(&cf));
        return *this;
    }

    /**
     * @brief Toggles the read-only state of the control.
     * @param ro  `true` to make the control read-only; `false` to allow editing.
     */
    richedit& set_readonly(bool ro) noexcept {
        SendMessageW(this->_hWnd, EM_SETREADONLY, ro ? TRUE : FALSE, 0);
        return *this;
    }
};

} // namespace wl
