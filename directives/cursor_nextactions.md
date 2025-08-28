next actions

CMake switches

Add cache vars: SCALAR_ONLY (OFF), ENABLE_SIMD (ON by default).

Two build presets: simd-on, scalar-only.

Non-AVX CI build uses -DENABLE_SIMD=OFF.

Wire main

main.cpp: parse --frames --fixed-dt --seed --profile --spawn --scalar.

Call sim::run(...).

Add missing stubs now

engine/ecs/index_map.{hpp,cpp}

engine/ecs/factory.{hpp,cpp}

engine/metrics/metrics.{hpp,cpp} (RAII timer)

Ensure headers compile and targets are in engine/CMakeLists.txt.

Tests harness

Doctest main in tests/unit/ecs_basic.cpp.

Targets: unit_tests, perf_smoke.

ctest presets mapped in CMakePresets.json.

Perf scripts

tools/perf/run_sweeps.{sh,bat} run:

engine_app --spawn 1000,10000,50000,100000 --frames 200 --fixed-dt 0.016 --profile on

Write CSVs under artifacts/perf/YYYYMMDD/.

CI matrix

Jobs:

Windows AVX2: build + unit tests + perf_smoke.

Linux scalar-only: build + unit tests.

Cache submodules. Fail CI on test failure.

Formatting + static checks

Add .clang-format.

Optional: clang-tidy target on engine_core.

Done for Cursor: both presets build, engine_app runs with flags, tests execute in CI, perf scripts produce CSV directories.

Gate to next phase

Determinism test passes.

SIMD ≡ scalar hash.

100k entities run completes headless with stable CSV.

No allocations detected in the hot loop.