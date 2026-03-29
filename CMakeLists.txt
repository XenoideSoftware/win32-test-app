cmake_minimum_required(VERSION 3.28)
project(ModernWinApp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(APP_UNICODE "Build with Unicode character set (default: ANSI)" OFF)

option(APP_ENABLE_HEXCTRL "Enable HexCtrl submodule (default: OFF)" OFF)
option(APP_ENABLE_PROPERTYGRID  "Enable PropertyGrid submodule (default: OFF)" OFF)

add_subdirectory(submodules)

add_executable(ModernWinApp WIN32 src/main.cpp)

target_compile_definitions(ModernWinApp PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    $<$<BOOL:${APP_UNICODE}>:UNICODE _UNICODE>
)

target_link_libraries(ModernWinApp PRIVATE
    user32 gdi32
    winlamb::winlamb
    mctrl::mctrl
    # win32-property-grid::win32-property-grid
    # $<TARGET_NAME_IF_EXISTS:HexCtrl::HexCtrl>
)

# Copy mCtrl.dll next to the executable after each build
add_custom_command(TARGET ModernWinApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:mCtrl>
        $<TARGET_FILE_DIR:ModernWinApp>
    COMMENT "Copying mCtrl.dll to output directory"
)
