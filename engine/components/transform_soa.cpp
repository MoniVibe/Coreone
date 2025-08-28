#include "transform_soa.hpp"
#include <algorithm>
#include <new>

namespace eng { namespace components {

TransformSoA::TransformSoA(std::pmr::monotonic_buffer_resource* resource)
  : memory(resource), allocator(resource) {}

TransformSoA::~TransformSoA() { deallocate(); }

TransformSoA::TransformSoA(TransformSoA&& other) noexcept
  : allocator(std::pmr::get_default_resource()) { moveFrom(std::move(other)); }

TransformSoA& TransformSoA::operator=(TransformSoA&& other) noexcept {
  if (this != &other) {
    deallocate();
    moveFrom(std::move(other));
  }
  return *this;
}

void* TransformSoA::allocateAligned(std::size_t size) {
  const std::size_t alignedSize = size + ALIGNMENT - 1;
  void* raw = allocator.allocate(alignedSize);
  const std::size_t addr = reinterpret_cast<std::size_t>(raw);
  const std::size_t aligned = (addr + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
  assert((aligned & (ALIGNMENT - 1)) == 0);
  return reinterpret_cast<void*>(aligned);
}

void TransformSoA::reserve(std::size_t n) {
  if (n <= capacity) return;
  n = (n + 15) & ~static_cast<std::size_t>(15);

  const std::size_t floatBytes = n * sizeof(float);

  float* newPosX = static_cast<float*>(allocateAligned(floatBytes));
  float* newPosY = static_cast<float*>(allocateAligned(floatBytes));
  float* newPosZ = static_cast<float*>(allocateAligned(floatBytes));

  float* newQuatX = static_cast<float*>(allocateAligned(floatBytes));
  float* newQuatY = static_cast<float*>(allocateAligned(floatBytes));
  float* newQuatZ = static_cast<float*>(allocateAligned(floatBytes));
  float* newQuatW = static_cast<float*>(allocateAligned(floatBytes));

  float* newScaleX = static_cast<float*>(allocateAligned(floatBytes));
  float* newScaleY = static_cast<float*>(allocateAligned(floatBytes));
  float* newScaleZ = static_cast<float*>(allocateAligned(floatBytes));

  if (count > 0) {
    std::memcpy(newPosX, posX, count * sizeof(float));
    std::memcpy(newPosY, posY, count * sizeof(float));
    std::memcpy(newPosZ, posZ, count * sizeof(float));

    std::memcpy(newQuatX, quatX, count * sizeof(float));
    std::memcpy(newQuatY, quatY, count * sizeof(float));
    std::memcpy(newQuatZ, quatZ, count * sizeof(float));
    std::memcpy(newQuatW, quatW, count * sizeof(float));

    std::memcpy(newScaleX, scaleX, count * sizeof(float));
    std::memcpy(newScaleY, scaleY, count * sizeof(float));
    std::memcpy(newScaleZ, scaleZ, count * sizeof(float));
  }

  posX = newPosX; posY = newPosY; posZ = newPosZ;
  quatX = newQuatX; quatY = newQuatY; quatZ = newQuatZ; quatW = newQuatW;
  scaleX = newScaleX; scaleY = newScaleY; scaleZ = newScaleZ;

  capacity = n;
  for (std::size_t i = count; i < capacity; ++i) {
    scaleX[i] = 1.0f; scaleY[i] = 1.0f; scaleZ[i] = 1.0f;
  }
}

void TransformSoA::deallocate() {
  posX = posY = posZ = nullptr;
  quatX = quatY = quatZ = quatW = nullptr;
  scaleX = scaleY = scaleZ = nullptr;
  capacity = count = 0;
}

void TransformSoA::moveFrom(TransformSoA&& other) noexcept {
  posX = other.posX; posY = other.posY; posZ = other.posZ;
  quatX = other.quatX; quatY = other.quatY; quatZ = other.quatZ; quatW = other.quatW;
  scaleX = other.scaleX; scaleY = other.scaleY; scaleZ = other.scaleZ;
  count = other.count; capacity = other.capacity; memory = other.memory;
  allocator.~polymorphic_allocator();
  new (&allocator) std::pmr::polymorphic_allocator<std::byte>(other.allocator.resource());
  other.posX = other.posY = other.posZ = nullptr;
  other.quatX = other.quatY = other.quatZ = other.quatW = nullptr;
  other.scaleX = other.scaleY = other.scaleZ = nullptr;
  other.count = other.capacity = 0;
  other.allocator.~polymorphic_allocator();
  new (&other.allocator) std::pmr::polymorphic_allocator<std::byte>(std::pmr::get_default_resource());
}

void TransformSoA::getPosition(std::size_t i, float& x, float& y, float& z) const {
  assert(valid(i)); x = posX[i]; y = posY[i]; z = posZ[i];
}
void TransformSoA::setPosition(std::size_t i, float x, float y, float z) {
  assert(valid(i)); posX[i] = x; posY[i] = y; posZ[i] = z;
}

void TransformSoA::getQuaternion(std::size_t i, float& x, float& y, float& z, float& w) const {
  assert(valid(i)); x = quatX[i]; y = quatY[i]; z = quatZ[i]; w = quatW[i];
}
void TransformSoA::setQuaternion(std::size_t i, float x, float y, float z, float w) {
  assert(valid(i)); quatX[i] = x; quatY[i] = y; quatZ[i] = z; quatW[i] = w;
}

void TransformSoA::getScale(std::size_t i, float& x, float& y, float& z) const {
  assert(valid(i)); x = scaleX[i]; y = scaleY[i]; z = scaleZ[i];
}
void TransformSoA::setScale(std::size_t i, float x, float y, float z) {
  assert(valid(i)); scaleX[i] = x; scaleY[i] = y; scaleZ[i] = z;
}

void TransformSoA::loadPositionAligned4(std::size_t i, float* dst) const {
  assert(valid(i + 3));
  assert(((uintptr_t)dst & 31) == 0);
  std::memcpy(dst + 0, posX + i, 4 * sizeof(float));
  std::memcpy(dst + 4, posY + i, 4 * sizeof(float));
  std::memcpy(dst + 8, posZ + i, 4 * sizeof(float));
}

void TransformSoA::storePositionAligned4(std::size_t i, const float* src) {
  assert(valid(i + 3));
  assert(((uintptr_t)src & 31) == 0);
  std::memcpy(posX + i, src + 0, 4 * sizeof(float));
  std::memcpy(posY + i, src + 4, 4 * sizeof(float));
  std::memcpy(posZ + i, src + 8, 4 * sizeof(float));
}

void TransformSoA::loadQuaternionAligned4(std::size_t i, float* dst) const {
  assert(valid(i + 3));
  assert(((uintptr_t)dst & 31) == 0);
  std::memcpy(dst + 0, quatX + i, 4 * sizeof(float));
  std::memcpy(dst + 4, quatY + i, 4 * sizeof(float));
  std::memcpy(dst + 8, quatZ + i, 4 * sizeof(float));
  std::memcpy(dst + 12, quatW + i, 4 * sizeof(float));
}

void TransformSoA::storeQuaternionAligned4(std::size_t i, const float* src) {
  assert(valid(i + 3));
  assert(((uintptr_t)src & 31) == 0);
  std::memcpy(quatX + i, src + 0, 4 * sizeof(float));
  std::memcpy(quatY + i, src + 4, 4 * sizeof(float));
  std::memcpy(quatZ + i, src + 8, 4 * sizeof(float));
  std::memcpy(quatW + i, src + 12, 4 * sizeof(float));
}

std::size_t TransformSoA::add(float px, float py, float pz,
                              float qx, float qy, float qz, float qw,
                              float sx, float sy, float sz) {
  if (count >= capacity) {
    reserve(capacity == 0 ? 64 : capacity * 2);
  }
  const std::size_t idx = count++;
  posX[idx] = px; posY[idx] = py; posZ[idx] = pz;
  quatX[idx] = qx; quatY[idx] = qy; quatZ[idx] = qz; quatW[idx] = qw;
  scaleX[idx] = sx; scaleY[idx] = sy; scaleZ[idx] = sz;
  return idx;
}

void TransformSoA::remove(std::size_t i) {
  assert(valid(i));
  if (i == count - 1) { --count; return; }
  const std::size_t last = count - 1;
  posX[i] = posX[last]; posY[i] = posY[last]; posZ[i] = posZ[last];
  quatX[i] = quatX[last]; quatY[i] = quatY[last]; quatZ[i] = quatZ[last]; quatW[i] = quatW[last];
  scaleX[i] = scaleX[last]; scaleY[i] = scaleY[last]; scaleZ[i] = scaleZ[last];
  --count;
}

void TransformSoA::verifyAlignment() const {
  auto check = [](const void* p) {
    const auto addr = reinterpret_cast<std::uintptr_t>(p);
    assert((addr & (ALIGNMENT - 1)) == 0);
  };
  check(posX); check(posY); check(posZ);
  check(quatX); check(quatY); check(quatZ); check(quatW);
  check(scaleX); check(scaleY); check(scaleZ);
}

} }


