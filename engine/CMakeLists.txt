add_library(engine_core
  ecs/ecs_register.cpp
  ecs/index_map.cpp
  ecs/factory.cpp
  ecs/chunk_iter.cpp
  components/transform_soa.cpp
  components/velocity_soa.cpp
  components/bounding_sphere_soa.cpp
  systems/system_integrate.cpp
  metrics/metrics.cpp
  jobs/job_system.cpp
  alloc/arena.cpp
  sim/sim_loop.cpp
  render/view_stub.cpp
)

target_include_directories(engine_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(engine_core PUBLIC flecs::flecs glm fmt)

if(ENABLE_AVX2 AND ENABLE_SIMD AND NOT SCALAR_ONLY)
  if(MSVC)
    target_compile_options(engine_core PRIVATE /arch:AVX2)
  else()
    target_compile_options(engine_core PRIVATE -mavx2 -mfma)
  endif()
  target_compile_definitions(engine_core PRIVATE ENABLE_SIMD=1)
else()
  target_compile_definitions(engine_core PRIVATE ENABLE_SIMD=0)
endif()

add_executable(engine_app ${CMAKE_SOURCE_DIR}/main.cpp)
target_link_libraries(engine_app PRIVATE engine_core)

if(ENABLE_RENDER)
  # Placeholder for bgfx linkage when enabled
endif()


