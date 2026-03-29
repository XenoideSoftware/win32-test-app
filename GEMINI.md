# GEMINI.md

This file provides guidance to Gemini when working with code in this repository.

## Build Commands

```bash
# Generate Visual Studio solution (run from repo root)
cmake -B build

# Build the project
cmake --build build

# Build a specific configuration
cmake --build build --config Debug
cmake --build build --config Release
```

Unicode support is off by default. To enable:
```bash
cmake -B build -DAPP_UNICODE=ON
```

## Architecture

This is a native Windows GUI application in C++17 used to test a miriad of external Win32 libraries, backed by several git submodules.

### Key libraries (in `submodules/`)

- **winlamb** — Header-only C++ wrapper over Win32 API. Windows inherit from `wl::window_main`, `wl::dialog_main`, etc. Message handlers are registered via lambda callbacks in the constructor. Use `RUN(ClassName)` macro to generate the `WinMain` entry point.
- **mctrl** — Compiled as a shared library (`mCtrl.dll`). Provides modern Win32 controls beyond standard USER32/COMCTL32. The DLL is automatically copied next to the executable at post-build.
- **HexCtrl** — Hex editor/viewer control using C++20 modules (`.ixx` files). Conditionally compiled only when CMake >= 3.28.
- **win32-property-grid** — Static library, compiled as C despite `.cpp` extension.

### WinLamb window pattern

```cpp
class My_Window : public wl::window_main {
public:
    My_Window() {
        on_message(WM_CREATE, [this](wl::params p) -> LRESULT {
            // setup controls here
            return 0;
        });
        on_message(WM_COMMAND, [this](wl::params p) -> LRESULT {
            // handle commands
            return 0;
        });
    }
};
RUN(My_Window)
```

### No test infrastructure in main app

Tests exist inside mCtrl submodule but are disabled (`MCTRL_BUILD_TESTS OFF`). There is no test runner for the main application.
