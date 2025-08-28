add_executable(unit_tests
  unit/test_main.cpp
  unit/ecs_basic.cpp
  unit/simd_equivalence.cpp
)

target_link_libraries(unit_tests PRIVATE engine_core doctest)
add_test(NAME unit_tests COMMAND unit_tests)

add_executable(perf_smoke perf/perf_smoke.cpp)
target_link_libraries(perf_smoke PRIVATE engine_core fmt)


