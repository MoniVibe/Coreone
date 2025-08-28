#include "factory.hpp"
#include "ecs_register.hpp"
#include <algorithm>

namespace eng { namespace ecs {

// Velocity component stored in ECS
struct Velocity { float x, y, z; };

// Radius component for collision/culling
struct Radius { float value; };

EntityFactory::EntityFactory(flecs::world& w,
                             eng::components::TransformSoA& transforms,
                             IndexMap& indexMap)
  : world(w), transformSoA(transforms), indexMap(indexMap) {
  world.component<Velocity>("Velocity");
  world.component<Radius>("Radius");
  world.component<Movable>("Movable");
}

flecs::entity EntityFactory::spawnMovable(Vec3 pos, Vec3 vel, float radius) {
  auto entity = world.entity()
    .add<Movable>()
    .set<Velocity>({vel.x, vel.y, vel.z})
    .set<Radius>({radius});

  const std::size_t soaIndex = transformSoA.add(
    pos.x, pos.y, pos.z,
    0.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f
  );

  (void)soaIndex; // reserved for future reverse mapping
  indexMap.add(entity);

  if (transformSoA.size() >= transformSoA.capacity * 3 / 4) {
    const std::size_t newCapacity = std::max<std::size_t>(64, transformSoA.capacity * 2);
    transformSoA.reserve(newCapacity);
  }

  return entity;
}

void EntityFactory::despawn(flecs::entity e) {
  const auto idxOpt = indexMap.get(e);
  if (idxOpt.has_value()) {
    const std::size_t idx = idxOpt.value();
    transformSoA.remove(idx);
    indexMap.remove(e);
    // Note: If we swapped with last, reverse mapping update would be needed.
  }
  e.destruct();
}

void EntityFactory::reserve(std::size_t expectedCount) {
  transformSoA.reserve(expectedCount);
  indexMap.reserve(expectedCount);
}

} }


