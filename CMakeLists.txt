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


