next actions

Bind SoA to Flecs

engine/ecs/ecs_register.{hpp,cpp}:

void register_components(flecs::world&);

Register tags: Movable, Debug.

engine/ecs/index_map.{hpp,cpp}:

Dense index map: Entity -> SoA index with free-list.

API: add(e) -> idx, remove(e), get(e)->idx.

Factories

engine/ecs/factory.{hpp,cpp}:

flecs::entity spawn_movable(flecs::world&, vec3 p0, vec3 v0, float r);

void despawn(flecs::entity e);

Ensure SoA capacity growth is amortized and aligned. No realloc in hot loop.

System: IntegrateMotion

Use chunk_iter ranges.

SIMD path default. Scalar path behind runtime flag --scalar.

Tail handling and alignment asserts done.

Job system integration

schedule_integrate(const Range*, int nRanges, float dt):

Split ranges via split_range.

Submit tasks to JobSystem, wait_all().

Sim loop

engine/sim/sim_loop.{hpp,cpp}:

void init(u64 seed, int spawnN);

void tick(float dt);

void run(int frames, float dt, bool profile);

Pipeline: Orders(noop) → IntegrateMotion → LOD(noop) → Culling(noop).

Determinism + checksums

u64 hash_positions(const TransformSoA&).

Record hash_after_frame_100 for tests.

Profiling + CSV

metrics.hpp/.cpp: RAII timer.

Emit frame,entities,ms_integrate,ms_total,simd_used,stolen_tasks to stdout when --profile on.

Tests

tests/unit/ecs_basic.cpp: spawn 10, run 10 frames, positions exact.

tests/unit/simd_equivalence.cpp: scalar vs SIMD hashes equal after 100 frames.

tests/perf/perf_smoke.cpp: 10k for 100 frames, assert ms_total/frame < budget placeholder.

Guards

Allocation counter in hot loop scope; assert zero deltas.

Compile-time ENABLE_SIMD and runtime --scalar override.

AVX2 feature check → fallback to scalar.

Done for Claude: headless app runs with --spawn N --frames F --fixed-dt S --profile on, tests green, CSV produced.

