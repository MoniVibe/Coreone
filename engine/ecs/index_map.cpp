#include "index_map.hpp"
#include <algorithm>

namespace eng { namespace ecs {

std::size_t IndexMap::add(flecs::entity e) {
  const auto id = e.id();

  const auto it = entity_to_index.find(id);
  if (it != entity_to_index.end()) {
    return it->second;
  }

  std::size_t index = 0;
  if (!free_indices.empty()) {
    index = free_indices.back();
    free_indices.pop_back();
  } else {
    index = next_index++;
  }

  entity_to_index[id] = index;
  return index;
}

void IndexMap::remove(flecs::entity e) {
  const auto id = e.id();
  const auto it = entity_to_index.find(id);
  if (it != entity_to_index.end()) {
    free_indices.push_back(it->second);
    entity_to_index.erase(it);
  }
}

std::optional<std::size_t> IndexMap::get(flecs::entity e) const {
  const auto id = e.id();
  const auto it = entity_to_index.find(id);
  if (it != entity_to_index.end()) {
    return it->second;
  }
  return std::nullopt;
}

bool IndexMap::contains(flecs::entity e) const {
  return entity_to_index.find(e.id()) != entity_to_index.end();
}

void IndexMap::clear() {
  entity_to_index.clear();
  free_indices.clear();
  next_index = 0;
}

void IndexMap::reserve(std::size_t capacity) {
  entity_to_index.reserve(capacity);
  free_indices.reserve(capacity / 4);
}

} }


