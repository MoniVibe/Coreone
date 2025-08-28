# Cursor Directive: Environment, File Structure, Stubs, and Paths

> Organization first. Prepare everything Claude needs **before** code.

## Scope
- Create a clean repo skeleton with predictable paths.
- Install and pin third‑party deps.
- Generate build presets and CI.
- Create stubs and TODOs so Claude can implement immediately.

## Target hosts
- **Windows**: VS2022 + Ninja
- **Linux**: clang‑16+ or gcc‑12+, Ninja

## Prerequisites
- Git ≥ 2.39
- CMake ≥ 3.27
- Ninja ≥ 1.11
- Python ≥ 3.10 (tools scripts)
- Visual Studio 2022 Build Tools (win) with C++ workload
- LLVM/Clang (optional but preferred on Linux)

---

## Repo root
Use the user’s projects folder.
```
C:\Users\Moni\Documents\claudeprojects\systems-foundations\
```
If different, adjust the paths in presets below.

**Initialize**
```bash
git init systems-foundations && cd systems-foundations
```

---

## Baseline files
Create at repo root:
- `README.md` – short description and build steps.
- `LICENSE` – placeholder (choose later).
- `.gitignore` – see template below.
- `.gitattributes` – normalize line endings.
- `.editorconfig` – whitespace policy.
- `.clang-format` – coding style (LLVM base, 120 cols).
- `CMakeLists.txt` – top level.
- `CMakePresets.json` – Windows/Linux presets.
- `.github/workflows/ci.yml` – build + tests.

Templates:
```gitignore
# build
/build/
/out/
/.cache/
CMakeFiles/
CMakeCache.txt
compile_commands.json

# tools
/artifacts/
/.vscode/
.idea/

# OS
.DS_Store
Thumbs.db
```

```gitattributes
* text=auto eol=lf
*.bat text eol=crlf
*.sln text eol=crlf
```

```editorconfig
root = true
[*]
charset = utf-8
end_of_line = lf
insert_final_newline = true
indent_style = space
indent_size = 2
trim_trailing_whitespace = true
[*.{bat,ps1}]
end_of_line = crlf
```

```yaml
# .github/workflows/ci.yml
name: ci
on: [push, pull_request]
jobs:
  build:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [windows-latest, ubuntu-latest]
    steps:
      - uses: actions/checkout@v4
        with: { submodules: recursive }
      - name: Configure
        run: cmake --preset ${{ runner.os == 'Windows' && 'win-msvc' || 'linux-clang' }}
      - name: Build
        run: cmake --build --preset ${{ runner.os == 'Windows' && 'win-msvc' || 'linux-clang' }} --target engine_app unit_tests -j 8
      - name: Test
        run: ctest --preset ${{ runner.os == 'Windows' && 'win-msvc' || 'linux-clang' }} --output-on-failure
```

---

## Directory layout
```
/engine
  /ecs                  # flecs wrappers, registration, chunk iterators
  /components           # SoA hot data, AoS cold data
  /systems              # integrate, lod, culling (stubs)
  /jobs                 # task graph + worker pool (stubs)
  /alloc                # arenas and pools (stubs)
  /sim                  # sim loop, config, snapshot
  /render               # bgfx later; stub now
  /simd                 # header-only SIMD wrappers
/tools
  /perf                 # run_sweeps scripts
  /inspect              # dump utilities
/tests
  /unit
  /perf
/third_party            # submodules pinned
/cmake                  # toolchain and helper cmake
/docs
  /directives
```

Create empty files with TODOs so headers include cleanly and build does not fail.

---

## Third‑party dependencies (submodules)
```bash
git submodule add https://github.com/SanderMertens/flecs third_party/flecs
git submodule add https://github.com/bkaradzic/bx third_party/bx
git submodule add https://github.com/bkaradzic/bimg third_party/bimg
git submodule add https://github.com/bkaradzic/bgfx third_party/bgfx
git submodule add https://github.com/g-truc/glm third_party/glm
git submodule add https://github.com/doctest/doctest third_party/doctest
git submodule add https://github.com/fmtlib/fmt third_party/fmt
git submodule add https://github.com/glfw/glfw third_party/glfw
```
Pin to known good commits (fill in later):
```bash
git -C third_party/flecs rev-parse HEAD  # record SHA in docs
# Example pin:
# git -C third_party/flecs checkout <sha>
```

Update instructions:
```bash
git submodule update --init --recursive
```

---

## CMake: top level
`CMakeLists.txt` minimal contract Claude will rely on.
```cmake
cmake_minimum_required(VERSION 3.27)
project(SystemsFoundations LANGUAGES CXX)

option(ENABLE_RENDER "Enable BGFX renderer" OFF)
option(ENABLE_TESTS  "Build unit tests" ON)
option(ENABLE_AVX2   "Enable AVX2" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Third party include dirs
add_subdirectory(third_party/flecs EXCLUDE_FROM_ALL)
# header-only libs
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE third_party/glm)
add_library(doctest INTERFACE)
target_include_directories(doctest INTERFACE third_party/doctest)
add_library(fmt INTERFACE)
target_include_directories(fmt INTERFACE third_party/fmt/include)

add_subdirectory(engine)
if(ENABLE_TESTS)
  add_subdirectory(tests)
endif()
```

### Engine CMake
`engine/CMakeLists.txt`
```cmake
add_library(engine_core
  ecs/ecs_register.cpp
  ecs/chunk_iter.cpp
  components/transform_soa.cpp
  components/velocity_soa.cpp
  components/bounding_sphere_soa.cpp
  systems/system_integrate.cpp
  jobs/job_system.cpp
  alloc/arena.cpp
  sim/sim_loop.cpp
  render/view_stub.cpp
)

target_include_directories(engine_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(engine_core PUBLIC flecs::flecs glm fmt)

if(ENABLE_AVX2)
  if(MSVC)
    target_compile_options(engine_core PRIVATE /arch:AVX2)
  else()
    target_compile_options(engine_core PRIVATE -mavx2 -mfma)
  endif()
  target_compile_definitions(engine_core PRIVATE ENABLE_SIMD=1)
endif()

add_executable(engine_app ${CMAKE_SOURCE_DIR}/main.cpp)
target_link_libraries(engine_app PRIVATE engine_core)

if(ENABLE_RENDER)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/bgfx ${CMAKE_BINARY_DIR}/bgfx EXCLUDE_FROM_ALL)
  # Link bgfx stack when render is enabled later
endif()
```

### Tests CMake
`tests/CMakeLists.txt`
```cmake
add_executable(unit_tests
  unit/ecs_basic.cpp
  unit/simd_equivalence.cpp
)

target_link_libraries(unit_tests PRIVATE engine_core doctest)
add_test(NAME unit_tests COMMAND unit_tests)

add_executable(perf_smoke perf/perf_smoke.cpp)
target_link_libraries(perf_smoke PRIVATE engine_core fmt)
```

### Presets
`CMakePresets.json`
```json
{
  "version": 5,
  "configurePresets": [
    {
      "name": "win-msvc",
      "generator": "Ninja",
      "binaryDir": "build/win-msvc",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "RelWithDebInfo",
        "ENABLE_TESTS": "ON",
        "ENABLE_RENDER": "OFF",
        "ENABLE_AVX2": "ON"
      }
    },
    {
      "name": "linux-clang",
      "generator": "Ninja",
      "binaryDir": "build/linux-clang",
      "cacheVariables": {
        "CMAKE_C_COMPILER": "clang",
        "CMAKE_CXX_COMPILER": "clang++",
        "CMAKE_BUILD_TYPE": "RelWithDebInfo",
        "ENABLE_TESTS": "ON",
        "ENABLE_RENDER": "OFF",
        "ENABLE_AVX2": "ON"
      }
    }
  ],
  "buildPresets": [
    { "name": "win-msvc", "configurePreset": "win-msvc" },
    { "name": "linux-clang", "configurePreset": "linux-clang" }
  ],
  "testPresets": [
    { "name": "win-msvc", "configurePreset": "win-msvc" },
    { "name": "linux-clang", "configurePreset": "linux-clang" }
  ]
}
```

---

## Stubs Claude expects
Create these files with minimal compilable content and `TODO(claude)` markers.

```
engine/ecs/ecs_register.{hpp,cpp}
engine/ecs/chunk_iter.{hpp,cpp}
engine/components/transform_soa.{hpp,cpp}
engine/components/velocity_soa.{hpp,cpp}
engine/components/bounding_sphere_soa.{hpp,cpp}
engine/systems/system_integrate.{hpp,cpp}
engine/jobs/job_system.{hpp,cpp}
engine/alloc/arena.{hpp,cpp}
engine/sim/sim_loop.{hpp,cpp}
engine/render/view_stub.{hpp,cpp}
engine/simd/simd.hpp
main.cpp
```

### Header skeleton example
`engine/components/transform_soa.hpp`
```cpp
#pragma once
#include <cstddef>
#include <cstdint>
namespace eng { struct TransformSoA { std::size_t count{0}; std::size_t capacity{0}; /* TODO(claude): arrays + api */ }; }
```

### CPP skeleton example
`engine/components/transform_soa.cpp`
```cpp
#include "transform_soa.hpp"
namespace eng { /* TODO(claude): impl */ }
```

### Main entry
`main.cpp`
```cpp
#include <cstdio>
int main(int argc, char** argv){
  // TODO(cursor): parse --frames, --fixed-dt, --seed, --profile, --spawn
  // TODO(cursor): call sim::run(...)
  std::puts("engine_app stub");
  return 0;
}
```

---

## Tools
- `tools/perf/run_sweeps.{sh,bat}`: iterate spawn sizes and write CSV under `artifacts/perf/`.
- `tools/inspect/dump_archetypes.{cpp,md}`: prints counts (stub until Claude wires ECS query).

Example `run_sweeps.sh`:
```bash
#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
APP="$ROOT/build/linux-clang/engine_app"
OUT="$ROOT/artifacts/perf/$(date +%Y%m%d)"
mkdir -p "$OUT"
for N in 1000 10000 50000 100000; do
  "$APP" --spawn $N --frames 200 --fixed-dt 0.016 --profile on > "$OUT/sweep_${N}.csv"
done
```

Windows `run_sweeps.bat` analogous with preset `win-msvc`.

---

## BGFX pathing (disabled now)
- Keep `ENABLE_RENDER=OFF` by default.
- Provide `engine/render/view_stub.hpp/.cpp` with no bgfx symbols so linkage is stable.
- Later enable and link `bx`, `bimg`, `bgfx` targets when renderer is turned on.

---

## Handoff checklist to Claude
- [ ] Repo builds `engine_app` and `unit_tests` with stubs only.
- [ ] Submodules initialized and pinned.
- [ ] Presets generate build dirs: `build/win-msvc`, `build/linux-clang`.
- [ ] CLI flags parsed and forwarded to `sim::run` stub.
- [ ] Placeholders compile for all headers listed above.
- [ ] CI green on both OS with stubs.

---

## Quickstart commands
```bash
# One-time
git submodule update --init --recursive
cmake --preset win-msvc   # or linux-clang
cmake --build --preset win-msvc -j 8
ctest --preset win-msvc --output-on-failure

# Perf scripts (after Claude adds logic)
./tools/perf/run_sweeps.sh
```

