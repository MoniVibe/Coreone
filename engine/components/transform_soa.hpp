#pragma once
#include <cstddef>
#include <cstdint>
#include <memory_resource>
#include <cassert>
#include <cstring>

namespace eng { namespace components {

// Structure-of-Arrays transform component for efficient SIMD processing
// All arrays are 32-byte aligned for AVX2 compatibility
struct TransformSoA {
  float* posX{nullptr};
  float* posY{nullptr};
  float* posZ{nullptr};

  float* quatX{nullptr};
  float* quatY{nullptr};
  float* quatZ{nullptr};
  float* quatW{nullptr};

  float* scaleX{nullptr};
  float* scaleY{nullptr};
  float* scaleZ{nullptr};

  std::size_t count{0};
  std::size_t capacity{0};

  std::pmr::monotonic_buffer_resource* memory{nullptr};
  std::pmr::polymorphic_allocator<std::byte> allocator{std::pmr::get_default_resource()};

  static constexpr std::size_t ALIGNMENT = 32;

  TransformSoA() = default;
  explicit TransformSoA(std::pmr::monotonic_buffer_resource* resource);
  ~TransformSoA();

  TransformSoA(const TransformSoA&) = delete;
  TransformSoA& operator=(const TransformSoA&) = delete;
  TransformSoA(TransformSoA&& other) noexcept;
  TransformSoA& operator=(TransformSoA&& other) noexcept;

  void reserve(std::size_t n);
  std::size_t size() const { return count; }
  bool valid(std::size_t i) const { return i < count; }

  void getPosition(std::size_t i, float& x, float& y, float& z) const;
  void setPosition(std::size_t i, float x, float y, float z);

  void getQuaternion(std::size_t i, float& x, float& y, float& z, float& w) const;
  void setQuaternion(std::size_t i, float x, float y, float z, float w);

  void getScale(std::size_t i, float& x, float& y, float& z) const;
  void setScale(std::size_t i, float x, float y, float z);

  void loadPositionAligned4(std::size_t i, float* dst) const;
  void storePositionAligned4(std::size_t i, const float* src);

  void loadQuaternionAligned4(std::size_t i, float* dst) const;
  void storeQuaternionAligned4(std::size_t i, const float* src);

  std::size_t add(float px, float py, float pz,
                  float qx, float qy, float qz, float qw,
                  float sx = 1.0f, float sy = 1.0f, float sz = 1.0f);

  void remove(std::size_t i);
  void clear() { count = 0; }
  void verifyAlignment() const;

private:
  void* allocateAligned(std::size_t size);
  void deallocate();
  void moveFrom(TransformSoA&& other) noexcept;
};

} }


