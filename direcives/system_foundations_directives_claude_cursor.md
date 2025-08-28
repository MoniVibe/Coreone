# /docs/directives/CLAUDE_FIRST_STEPS.md

## Title
Foundations Coding Plan (Flecs + SoA + SIMD + Headless Sim)

## Role
Claude (Opus) writes production‑grade code. Cursor wires files, CMake, deps, and stubs.

## Objectives (v0)
- Deterministic headless simulation loop with fixed `dt`.
- SoA component storage for hot data.
- Scalar and SIMD implementations with equivalence tests.
- Minimal ECS pipeline with one real system: IntegrateMotion.
- Perf + determinism + checksum tests green in CI.

## Constraints
- No renderer in v0. BGFX stays stubbed.
- No heap allocs in hot loop. Use arenas/pools.
- Reproducible runs: fixed seeds. Stable system order.
- Code style: POD components, explicit alignment, no virtuals in hot path.

## Deliverables
1) `TransformSoA`, `VelocitySoA`, `BoundingSphereSoA` with aligned arrays.
2) `IntegrateMotion` system with scalar + SIMD paths.
3) `ChunkIterator` that iterates archetype blocks by range.
4) `JobSystem` minimal thread pool + range tasks (coarse v0).
5) Tests: determinism, SIMD≡scalar checksum, perf smoke CSV.

---

## Step 1 — SoA components
Create SoA headers in `engine/components/`:
- `transform_soa.hpp/.cpp`
  - Layout:
    ```cpp
    struct TransformSoA {
      // N = capacity of chunk
      float *posX, *posY, *posZ;      // 32-byte aligned
      float *quatX, *quatY, *quatZ, *quatW;
      float *scaleX, *scaleY, *scaleZ; // optional, default 1
      size_t count, capacity;
    };
    ```
  - API: `reserve(N)`, `size()`, `data(i)` getters, `loadAligned(i)`, `storeAligned(i)`.
- `velocity_soa.hpp/.cpp`: `vx[], vy[], vz[]` with same alignment.
- `bounding_sphere_soa.hpp/.cpp`: `r[]`.
- Memory: use `std::pmr::monotonic_buffer_resource` per chunk; assert 32‑byte alignment.

### SIMD wrappers
- `simd.hpp`: thin wrappers selecting SSE2/AVX2 at compile time. Provide:
  - `load4(float* p)`, `store4(float* p, vec4 v)`
  - `fma4(a,b,c)`, `mul4`, `add4`
  - Fallback scalar path under `#if !defined(__AVX2__)`.

---

## Step 2 — ECS registration and factories
- `ecs_register.hpp/.cpp`: register SoA components as Flecs components (opaque handles). Expose tags: `Tag_Movable`, `Tag_Debug`.
- `factory.hpp/.cpp`:
  - `Entity spawn_movable(vec3 p0, vec3 v0, float r)`
  - `void despawn(Entity e)` places indices into a free list; do not shrink SoA capacity in v0.

---

## Step 3 — Chunk iteration
- `chunk_iter.hpp`:
  - Define a fixed chunk size `CS = 4096` items.
  - Map SoA arrays into chunk ranges `[begin, end)`.
  - API: `for_each_chunk(count, CS, [&](Range r){ /* operate r.begin..r.end-1 */ });`
  - Provide `split_range(Range, workers)` for parallelization later.

---

## Step 4 — Systems
- `system_integrate.hpp/.cpp`:
  - Scalar path:
    ```cpp
    for (i = r.begin; i < r.end; ++i) {
      posX[i] += vx[i] * dt; posY[i] += vy[i] * dt; posZ[i] += vz[i] * dt;
    }
    ```
  - SIMD path (4‑wide minimum): load4/store4 over SoA lanes. Handle tail with scalar.
  - Checksum utility: `uint64 hash_positions(const TransformSoA&)` combining lane bits to verify equivalence.

- Pipeline order for v0: `Orders -> IntegrateMotion -> (noop LOD) -> (noop Culling)`.

---

## Step 5 — Job System (minimal)
- `job_system.hpp/.cpp`:
  - Fixed thread count = hardware_concurrency.
  - Per‑thread deque, global queue, work‑stealing when local empty.
  - Task type: `struct Task { Range r; void(*fn)(Range, void* ctx); void* ctx; };`
  - API: `submit(Task)`, `wait_all()`.

- `schedule_integrate()` splits chunks into tasks and submits.

---

## Step 6 — Sim loop
- `sim_loop.hpp/.cpp`:
  - `init(seed)` -> registers ECS, spawns N from CLI.
  - `tick(dt)` -> runs pipeline; records timers.
  - `run(frames, dt)` -> fixed‑step loop. No time drift.

- Output per frame: `frame, entities, ms_system_integrate, ms_total` to CSV when `--profile on`.

---

## Step 7 — Tests
- Determinism: run twice with same seed; compare `hash_positions` after 100 frames.
- SIMD equivalence: run scalar vs SIMD; assert equal hash.
- Perf smoke: N=10k for 100 frames; ensure frame time < budget; emit CSV.

---

## Step 8 — Error handling and asserts
- Alignment asserts on all SoA arrays.
- Guard SIMD with `#ifdef` checks and runtime fallback if unaligned/odd sizes.
- Zero allocations in the hot loop. Add counter to detect accidental allocations.

---

## CLI assumptions (Cursor wires args)
- `--frames N`, `--fixed-dt S`, `--seed X`, `--profile on|off`, `--spawn N`.

---

## Done criteria for v0
- Tests green locally and in CI.
- 100k entities step at 60 Hz on target CPU in headless mode (non‑binding but tracked).
- CSV emitted and parsed by tools script.



# /docs/directives/CURSOR_FIRST_STEPS.md

## Title
Repo Scaffolding, Dependencies, Build, and Tooling (Flecs + BGFX)

## Role
Cursor owns file structure, build system, placeholders, third‑party setup, scripts, and CI. Claude writes code into stubs you create.

## Repository layout
```
/engine
  /ecs
  /components
  /systems
  /jobs
  /alloc
  /sim
  /render
/tools
  /perf
  /inspect
/tests
  /unit
  /perf
/cmake
/docs/directives
```

## Initial files to create (empty or with TODO)
- `engine/ecs/ecs_register.{hpp,cpp}`
- `engine/ecs/chunk_iter.{hpp,cpp}`
- `engine/components/transform_soa.{hpp,cpp}`
- `engine/components/velocity_soa.{hpp,cpp}`
- `engine/components/bounding_sphere_soa.{hpp,cpp}`
- `engine/systems/system_integrate.{hpp,cpp}`
- `engine/jobs/job_system.{hpp,cpp}`
- `engine/alloc/arena.{hpp,cpp}` (simple PMR wrapper)
- `engine/sim/sim_loop.{hpp,cpp}`
- `engine/render/view_stub.{hpp,cpp}` (compiles, no BGFX calls yet)
- `engine/simd/simd.{hpp}` (headers only)
- `main.cpp` (CLI args only)

## Third‑party dependencies
Prefer git submodules pinned to known commits. Create `third_party/` with:
- `flecs/` (ECS)
- `bgfx/`, `bx/`, `bimg/` (renderer stack) — used later, compile off by default
- `glm/` (math)
- `doctest/` (tests)
- `fmt/` (logging/format)
- `glfw/` (window; off by default)

### Submodule commands
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
Pin revisions in `.gitmodules` or via `git -C third_party/<lib> checkout <commit>`.

## CMake scaffolding
Top‑level `CMakeLists.txt`:
- Project options:
  - `ENABLE_RENDER` (default OFF)
  - `ENABLE_TESTS` (default ON)
  - `ENABLE_AVX2` (detect and set compile flags)
- Add subdirs: `engine`, `tests` when enabled.
- Include third_party headers with `target_include_directories`.
- Define `engine_core` static lib with all `engine/*` except `render`.
- Define `engine_app` executable linking `engine_core` and conditionally `render_bgfx` later.
- Compiler flags:
  - MSVC: `/O2 /permissive- /arch:AVX2` when enabled, `/fp:fast` for perf builds.
  - Clang/GCC: `-O3 -march=native` or `-mavx2 -mfma` when enabled.

`engine/CMakeLists.txt` should:
- Create targets: `engine_core`, `engine_render_stub`.
- Add `target_compile_definitions(engine_core PRIVATE ENABLE_SIMD=1)` when AVX2.

## Build presets
`CMakePresets.json` with `windows-msvc` and `linux-clang`.

## CLI wiring in `main.cpp`
Add flags: `--frames`, `--fixed-dt`, `--seed`, `--profile`, `--spawn`.
Pass to `sim::run(frames, dt, seed, spawn, profile)`.

## Testing setup
- Add `tests/CMakeLists.txt` to build `unit_tests` using doctest.
- Create files:
  - `tests/unit/ecs_basic.cpp`
  - `tests/unit/simd_equivalence.cpp`
  - `tests/perf/perf_smoke.cpp`
- Provide doctest main and empty test cases with TODO markers.

## Tools
Create scripts to automate perf runs and artifact dumps.
- `tools/perf/run_sweeps.sh` and `.bat`:
  - run `engine_app --spawn {1k,10k,50k,100k} --frames 200 --fixed-dt 0.016 --profile on`
  - write CSV to `artifacts/perf/YYYYMMDD/`.
- `tools/inspect/dump_archetypes.{cpp,md}`: placeholder that prints counts once Claude fills query.

## CI
- GitHub Actions `.github/workflows/ci.yml`:
  - Matrix: `windows-latest` (MSVC), `ubuntu-latest` (clang)
  - Steps: checkout, submodules, configure, build, run unit tests
  - Perf job optional behind `RUN_PERF=1`

## Compiler/SDK assumptions
- Windows: VS2022, CMake, Ninja, Python 3 for scripts.
- Linux: clang-16+, libpthread, libc++ optional.

## Renderer toggle
- Keep `ENABLE_RENDER=OFF` default. `engine/render/view_stub` compiled so app links.
- Add placeholder `render_bgfx.{hpp,cpp}` target behind the flag for later.

## Done criteria for scaffolding PR
- Configures and builds on Windows and Linux without code from Claude beyond stubs.
- Unit test target builds and runs with placeholder tests.
- Perf scripts execute app and produce empty or stub CSV.

## Handoff checklist to Claude
- All headers compile. Namespaces exist. Empty function bodies or TODOs present.
- CMake targets and include dirs wired. Compiler flags set.
- CLI parses and calls stubbed `sim::run` with default values.
- Submodules synced and pinned.

