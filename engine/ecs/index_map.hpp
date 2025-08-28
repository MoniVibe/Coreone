#pragma once

#include <flecs.h>
#include <vector>
#include <unordered_map>
#include <cstddef>
#include <optional>

namespace eng { namespace ecs {

// Dense index map for Entity -> SoA index mapping with free-list
class IndexMap {
public:
    IndexMap() = default;
    ~IndexMap() = default;
    
    // Add entity, return assigned index
    size_t add(flecs::entity e);
    
    // Remove entity, return its index to free list
    void remove(flecs::entity e);
    
    // Get index for entity, returns nullopt if not found
    std::optional<size_t> get(flecs::entity e) const;
    
    // Check if entity exists in map
    bool contains(flecs::entity e) const;
    
    // Get total number of active mappings
    size_t size() const { return entity_to_index.size(); }
    
    // Clear all mappings and reset state
    void clear();
    
    // Reserve capacity to avoid reallocation
    void reserve(size_t capacity);
    
private:
    // Entity to SoA index mapping
    std::unordered_map<flecs::id_t, size_t> entity_to_index;
    
    // Free list of available indices
    std::vector<size_t> free_indices;
    
    // Next index to allocate if free list is empty
    size_t next_index = 0;
};

} }
