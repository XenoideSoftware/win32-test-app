cmake_minimum_required(VERSION 3.28)
project(ModernWinApp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(APP_UNICODE "Build with Unicode character set (default: ANSI)" OFF)

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
    # win32-property-grid::win32-property-grid
    # $<TARGET_NAME_IF_EXISTS:HexCtrl::HexCtrl>
)
