#include "metrics.hpp"
#include <iomanip>

namespace eng { namespace profiling {

ScopedTimer::ScopedTimer(const std::string& n, bool e)
  : name(n), enabled(e), stopped(false) {
  if (enabled) {
    start = std::chrono::high_resolution_clock::now();
  }
}

ScopedTimer::~ScopedTimer() {
  if (enabled && !stopped) {
    stop();
  }
}

double ScopedTimer::elapsed_ms() const {
  if (!enabled) return 0.0;
  const auto now = std::chrono::high_resolution_clock::now();
  return std::chrono::duration<double, std::milli>(now - start).count();
}

void ScopedTimer::stop() {
  if (!enabled || stopped) return;
  const double elapsed = elapsed_ms();
  std::cerr << "[TIMER] " << name << ": "
            << std::fixed << std::setprecision(3)
            << elapsed << " ms" << std::endl;
  stopped = true;
}

thread_local std::size_t AllocationGuard::globalAllocationCount = 0;

AllocationGuard::AllocationGuard() {
  initialCount = globalAllocationCount;
  allocationCount = 0;
}

AllocationGuard::~AllocationGuard() {
  allocationCount = globalAllocationCount - initialCount;
  if (allocationCount > 0) {
    std::cerr << "[ALLOC] Detected " << allocationCount
              << " allocations in guarded scope!" << std::endl;
  }
}

void AllocationGuard::increment_allocation() { ++globalAllocationCount; }
void AllocationGuard::decrement_allocation() {
  if (globalAllocationCount > 0) --globalAllocationCount;
}

void MetricsCSV::write_header() {
  std::cout << "frame,entities,ms_integrate,ms_total,simd_used,stolen_tasks" << std::endl;
}

void MetricsCSV::write_row(int frame,
                           std::size_t entities,
                           double ms_integrate,
                           double ms_total,
                           bool simd_used,
                           int stolen_tasks) {
  std::cout << frame << ","
            << entities << ","
            << std::fixed << std::setprecision(3)
            << ms_integrate << ","
            << ms_total << ","
            << (simd_used ? 1 : 0) << ","
            << stolen_tasks
            << std::endl;
}

} }


