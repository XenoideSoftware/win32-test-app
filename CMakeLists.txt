cmake_minimum_required(VERSION 3.20)
project(ModernWinApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ModernWinApp WIN32 src/main.cpp)

target_compile_definitions(ModernWinApp PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

target_link_libraries(ModernWinApp PRIVATE user32 gdi32)
