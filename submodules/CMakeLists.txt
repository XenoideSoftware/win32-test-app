# ── winlamb ──────────────────────────────────────────────────────────────────
# Header-only C++ Win32 utility wrappers
add_library(winlamb INTERFACE)
target_include_directories(winlamb INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/
)
target_link_libraries(winlamb INTERFACE comctl32)
add_library(winlamb::winlamb ALIAS winlamb)

if (APP_ENABLE_PROPERTYGRID)
    # ── win32-property-grid ───────────────────────────────────────────────────────
    # Plain C source despite the .cpp extension
    add_library(win32-property-grid STATIC
        win32-property-grid/propertyGrid.cpp
    )
    set_source_files_properties(
        win32-property-grid/propertyGrid.cpp
        PROPERTIES LANGUAGE C
    )
    target_include_directories(win32-property-grid PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/win32-property-grid
    )
    target_link_libraries(win32-property-grid PUBLIC
        user32 comctl32 gdi32
    )
    add_library(win32-property-grid::win32-property-grid ALIAS win32-property-grid)
endif()

if (APP_ENABLE_MCTRL)
    # ── mCtrl ────────────────────────────────────────────────────────────────────
    # Shared (DLL) library of modern Win32 controls
    set(MCTRL_BUILD_EXAMPLES OFF CACHE BOOL "Disable mCtrl examples" FORCE)
    set(MCTRL_BUILD_TESTS    OFF CACHE BOOL "Disable mCtrl tests"    FORCE)
    # Bundled hsluv-c declares cmake_minimum_required(3.1); silence the removal error.
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
    add_subdirectory(mctrl)
    unset(CMAKE_POLICY_VERSION_MINIMUM)
    add_library(mctrl::mctrl ALIAS mCtrl)
endif()

if (APP_ENABLE_HEXCTRL)
    # ── HexCtrl ───────────────────────────────────────────────────────────────────
    # C++ 17 library
    add_library(HexCtrl STATIC
        HexCtrl/HexCtrl/src/res/HexCtrl.rc
    )

    target_sources(HexCtrl
        PRIVATE
            HexCtrl/HexCtrl/src/dep/HexCtrl_StrToNum.ixx
            HexCtrl/HexCtrl/src/dep/HexCtrl_ListEx.ixx
            HexCtrl/HexCtrl/src/HexUtility.ixx
            HexCtrl/HexCtrl/src/CHexScroll.ixx
            HexCtrl/HexCtrl/src/CHexSelection.ixx
            HexCtrl/HexCtrl/src/CHexDlgBkmMgr.ixx
            HexCtrl/HexCtrl/src/CHexDlgCodepage.ixx
            HexCtrl/HexCtrl/src/CHexDlgDataInterp.ixx
            HexCtrl/HexCtrl/src/CHexDlgGoTo.ixx
            HexCtrl/HexCtrl/src/CHexDlgModify.ixx
            HexCtrl/HexCtrl/src/CHexDlgProgress.ixx
            HexCtrl/HexCtrl/src/CHexDlgSearch.ixx
            HexCtrl/HexCtrl/src/CHexDlgTemplMgr.ixx
            HexCtrl/HexCtrl/src/CHexCtrl.ixx
    )

    set_target_properties(HexCtrl PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )

    target_compile_options(HexCtrl PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/utf-8 /W3 /experimental:module>
    )

    target_include_directories(HexCtrl PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/HexCtrl
    )

    target_link_libraries(HexCtrl PUBLIC
        user32 comctl32 gdi32
    )

    add_library(HexCtrl::HexCtrl ALIAS HexCtrl)

endif()
