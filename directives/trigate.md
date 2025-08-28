Proceed in three gates: validate → visibility → expand.

Gate 1 — Validate what exists (now)

Determinism: run --test twice and compare final hashes; add doctest that fails on mismatch.

SIMD≡Scalar: run once with --scalar and once without; assert identical hash after 100 frames.

Scaling: sweep --threads {1,2,4,8,12,16} at --spawn {10k,50k,100k}; record speedup. Stop if <1.5× from 1→4.

Alloc guard: ensure zero hot-loop allocs at 100k×200 frames.

Tail + alignment: fuzz --spawn over irregular sizes (e.g., primes) to hit all tails.

Gate 2 — Visibility without coupling
Claude

Snapshot buffer:

engine/sim/snapshot.{hpp,cpp}: double-buffered, read-only view of TransformSoA + optional LOD tags.

API: publish() at end of frame; lock-free swap.

CPU camera + frustum:

engine/systems/camera.hpp struct; frustum_planes(); all SoA-friendly.

Culling + LOD (CPU):

system_lod.{hpp,cpp}: distance bands → TagLOD{0,1,2}.

system_cull.{hpp,cpp}: frustum test → TagVisible.

Output counts to metrics.

Cursor

Add engine/sim/snapshot.* and new system files to CMake.

CLI: --camera \"px,py,pz|tx,ty,tz|fov,near,far\" and --lod-bands \"d0,d1,d2\".

Perf scripts: dump visible_count and lod_histogram columns.

Gate 3 — Minimal BGFX view (optional after Gate 2 passes)
Claude

engine/render/view_bgfx.{hpp,cpp} behind ENABLE_RENDER.

One pipeline:

Instanced boxes or points for TagVisible.

Per-instance transform from snapshot only. No sim reads from GPU.

Selection helpers:

engine/selection/ray.{hpp,cpp}: NDC→ray, ray-sphere test using SoA batch.

Cursor

Wire bgfx + bx + bimg in CMake when ENABLE_RENDER=ON.

CLI: --renderer gl|vk|d3d11|none, --draw mode=points|boxes.

CI: keep ENABLE_RENDER=OFF by default; add an opt-in job.

Atomic tasks by agent
Claude — implement

snapshot.{hpp,cpp}: double buffer, struct of spans to SoA arrays + LOD tags; publish() uses sequence id.

camera.hpp + frustum.hpp: plane extraction, distance test; SIMD batch if trivial.

system_lod and system_cull: chunk-parallel, write tags; metrics for counts.

metrics: add counters visible, lod0/1/2, culled.

selection/ray: ray build + ray-sphere batch test; checksum test.

Unit tests:

lod_bands_assign_correctly with synthetic rings.

frustum_cull_symmetry (rotate camera 90°, counts shift predictably).

snapshot_stable_read (reader sees consistent data while writer advances).

Cursor — prepare

CMake targets for new files; add ENABLE_RENDER option and bgfx link path guarded.

CLI parsing for camera and LOD; defaults: fov=60, near=0.1, far=10k, bands 100,1000,5000.

Perf scripts update to emit CSV with: frame,entities,ms_integrate,ms_lod,ms_cull,ms_total,visible,lod0,lod1,lod2,stolen_tasks.

CI:

Matrix: win-avx2, linux-scalar.

Cache submodules.

Upload artifacts/perf/*.csv on RUN_PERF=1.

Docs:

/docs/perf/baselines.md with today’s hashes and timings.

Update /docs/directives with new flags.

Exit criteria before any “real” rendering

Determinism and SIMD equivalence tests pass in CI.

At 100k entities: total frame < 16.6 ms headless on your target CPU, and culling+LOD < 10% of frame.

Snapshot shows consistent counts and no data races under --threads N.