Systems Foundations Skeleton

This repository provides a clean skeleton for Claude to implement systems and engine code on top of. It includes:
- CMake presets for Windows (MSVC) and Linux (Clang)
- Stubbed engine directories and minimal build targets
- Third-party submodules (flecs, glm, doctest, fmt, glfw, bgfx stack placeholders)
- CI workflow to build and run tests

Quickstart

1) Configure
```
cmake --preset win-msvc
```
2) Build
```
cmake --build --preset win-msvc -j 8
```
3) Test
```
ctest --preset win-msvc --output-on-failure
```

See docs in `direcives/cursor_env_setup.md` for full layout and tasks.


