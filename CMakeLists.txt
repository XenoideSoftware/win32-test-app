cmake_minimum_required(VERSION 3.28)
project(ModernWinApp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(APP_UNICODE "Build with Unicode character set (default: ANSI)" OFF)

option(APP_ENABLE_HEXCTRL "Enable HexCtrl submodule (default: OFF)" OFF)
option(APP_ENABLE_PROPERTYGRID  "Enable PropertyGrid submodule (default: OFF)" OFF)
option(APP_ENABLE_MCTRL  "Enable mCtrl submodule (default: OFF)" OFF)

add_subdirectory(submodules)

add_executable(ModernWinApp WIN32
    src/main.cpp
    src/wrappers/richedit.h
    src/wrappers/tabctrl.h
    src/wrappers/webview.h
    src/main_window.h
    src/left_panel.h
    src/tab_page_richtext.h
    src/tab_page_webview.h
    src/resource.h

    src/defer_window_pos.h
    src/splitter_base.h
    src/splitter_horizontal.h
    src/splitter_vertical.h

    src/app.rc

    submodules/winlamb/win10.exe.manifest
)

target_compile_definitions(ModernWinApp PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

if (APP_UNICODE)
    target_compile_definitions(ModernWinApp PRIVATE -DUNICODE -D_UNICODE)
    target_compile_definitions(ModernWinApp PRIVATE UNICODE _UNICODE)    
else ()
    message ("Building with ANSI character set")

    # Remove UNICODE and _UNICODE from ModernWinApp's private definitions
    get_target_property(defs ModernWinApp COMPILE_DEFINITIONS)
    if(defs)
        list(FILTER defs EXCLUDE REGEX "UNICODE(=.*)?")
        list(FILTER defs EXCLUDE REGEX "_UNICODE(=.*)?")

        message("defs: ${defs}")

        set_property(TARGET ModernWinApp PROPERTY COMPILE_DEFINITIONS ${defs})
    endif()
endif()

target_link_libraries(ModernWinApp PRIVATE
    user32 gdi32
    winlamb::winlamb
    # mctrl::mctrl
    # win32-property-grid::win32-property-grid
    # $<TARGET_NAME_IF_EXISTS:HexCtrl::HexCtrl>
)

if (APP_ENABLE_MCTRL)
    # Copy mCtrl.dll next to the executable after each build
    add_custom_command(TARGET ModernWinApp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:mCtrl>
            $<TARGET_FILE_DIR:ModernWinApp>
        COMMENT "Copying mCtrl.dll to output directory"
    )
endif()
