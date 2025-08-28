### Message to ChatGPT (for Claude to implement next)

- **Current status**
  - Build passes on Windows MSVC with Ninja. Flags supported: `--frames`, `--fixed-dt`, `--seed`, `--spawn`, `--profile=on|off`, `--scalar`.
  - Added options/presets: `ENABLE_SIMD`, `SCALAR_ONLY`; presets `win-msvc` and `win-msvc-scalar` (and Linux variants in presets).
  - Implemented: `eng::components::TransformSoA`, `eng::ecs::IndexMap`, `eng::ecs::EntityFactory`, `eng::profiling::{ScopedTimer, AllocationGuard, MetricsCSV}`. CMake wired.

- **Please generate code (as files in `claudeoutput/`) matching these names and namespaces**
  - Simulation loop
    - `sim_loop_hpp.txt`: `namespace eng::sim { struct Config; void run(const Config&); }`
    - `sim_loop_cpp.txt`: Fixed-timestep loop:
      - On startup: create `flecs::world`, construct `TransformSoA`, `IndexMap`, `EntityFactory`, pre-reserve `spawn`.
      - Spawn `cfg.spawn` `Movable` entities with randomized velocities (seeded by `cfg.seed`) and small-range positions.
      - Per frame: run `systems::integrate(dt)` to update positions; emit one CSV row with `MetricsCSV` when `cfg.profile` is on.
      - No allocations in the hot loop. Use pre-reserved capacity only. Wrap hot loop with `AllocationGuard` and ensure zero allocations.
  - Integrate system
    - `system_integrate.txt` (or split `system_integrate.{hpp,cpp}`) under `eng::systems`:
      - Read/write `eng::components::TransformSoA` positions; advance by velocity × `dt`.
      - Provide two paths: SIMD when `ENABLE_SIMD==1`, scalar when `ENABLE_SIMD==0`. Use SoA arrays; avoid STL in the hot loop.
  - SIMD helpers (optional)
    - `simd_wrapper.txt`: helpers wrapped in `#if ENABLE_SIMD` for aligned loads/stores; scalar fallback otherwise.
  - Job system (optional but preferred)
    - Use existing `engine/jobs/job_system.{hpp,cpp}` stubs; implement simple parallel-for with chunking. Deterministic: fixed chunk order, no work-stealing given fixed `seed`.
  - Determinism and equivalence tests
    - Extend tests in `tests/unit/` (output as `*.txt` ready to paste):
      - `simd_equivalence.cpp`: spawn fixed set under SIMD and scalar builds, run N frames, hash `TransformSoA` positions, ensure hashes match.
      - `ecs_basic.cpp`: create/destroy `Movable` via `EntityFactory`, verify `IndexMap` remains consistent after removals/swaps.
  - Main wiring
    - `main_app_cpp.txt`: parse existing flags, call `eng::sim::run(cfg)`, print a one-line summary (frames, entities, mean ms) on exit.
  - Formatting and CI
    - `.clang-format` (LLVM base, 2 spaces, sorted includes).
    - Optional `.clang-tidy` tuned for performance/modernize.
    - `github_actions_workflow.txt`: CI matrix:
      - Windows (SIMD on): configure+build+unit tests+run `tests/perf/perf_smoke.exe`.
      - Linux (scalar-only): configure+build+unit tests.
      - Cache third_party; fail on test failure.

- **Constraints and conventions**
  - Namespace must be `eng::...` everywhere.
  - Use `eng::components::TransformSoA`, `eng::ecs::{IndexMap, EntityFactory, Movable}`.
  - Avoid allocations in the hot loop; pre-reserve capacity and reuse buffers.
  - CSV schema is fixed: `frame,entities,ms_integrate,ms_total,simd_used,stolen_tasks`.
  - Determinism: identical results for same `seed`. SIMD and scalar must produce identical hashes.

- **Deliverables format**
  - Place each file’s content in `claudeoutput/` with the exact names above.
  - Use only current third_party (flecs, glm, fmt) and existing code. No new deps.


### Repo: Push current workspace to GitHub (owner: `MoniVibe`)

- Commands (PowerShell)
  - Using GitHub CLI (create and push):
```
cd "C:\Users\Moni\Documents\Claudeprojects\coreone"
git init
git checkout -b main
git add .
git commit -m "chore(repo): initial commit"

gh auth status || gh auth login
gh repo create "MoniVibe/coreone" --private --source . --remote origin --push
```
  - Manual Git (if you create the repo in the browser first):
```
cd "C:\Users\Moni\Documents\Claudeprojects\coreone"
git init
git checkout -b main
git add .
git commit -m "chore(repo): initial commit"

git remote add origin https://github.com/MoniVibe/coreone.git
git push -u origin main
```

- Optional: ignore local build outputs before the first commit
```
@"
build/
artifacts/
"@ | Out-File -Encoding utf8 -Append .gitignore
```

