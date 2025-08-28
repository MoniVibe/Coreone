#pragma once

#include <flecs.h>
#include "../components/transform_soa.hpp"
#include "index_map.hpp"

namespace eng { namespace ecs {

// Simple 3D vector for API
struct Vec3 {
  float x{0};
  float y{0};
  float z{0};
};

// Forward declaration for tag (defined in ecs_register.hpp)
struct Movable;

// Factory functions for entity creation/destruction
class EntityFactory {
public:
  EntityFactory(flecs::world& world,
                eng::components::TransformSoA& transforms,
                IndexMap& indexMap);

  // Spawn movable entity with position, velocity, radius
  flecs::entity spawnMovable(Vec3 pos, Vec3 vel, float radius);

  // Despawn entity and clean up resources
  void despawn(flecs::entity e);

  // Reserve capacity for expected entity count
  void reserve(std::size_t expectedCount);

private:
  flecs::world& world;
  eng::components::TransformSoA& transformSoA;
  IndexMap& indexMap;
};

} }


