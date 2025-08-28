#pragma once

#include <chrono>
#include <string>
#include <iostream>

namespace eng { namespace profiling {

// RAII timer for profiling code blocks
class ScopedTimer {
public:
  explicit ScopedTimer(const std::string& name, bool enabled = true);
  ~ScopedTimer();

  double elapsed_ms() const;
  void stop();

private:
  std::string name;
  std::chrono::high_resolution_clock::time_point start;
  bool enabled{true};
  bool stopped{false};
};

// Allocation counter for detecting allocations in hot paths
class AllocationGuard {
public:
  AllocationGuard();
  ~AllocationGuard();

  bool has_allocations() const { return allocationCount > 0; }
  std::size_t get_count() const { return allocationCount; }

  static void increment_allocation();
  static void decrement_allocation();

private:
  std::size_t initialCount{0};
  std::size_t allocationCount{0};
  static thread_local std::size_t globalAllocationCount;
};

class MetricsCSV {
public:
  static void write_header();
  static void write_row(int frame,
                        std::size_t entities,
                        double ms_integrate,
                        double ms_total,
                        bool simd_used,
                        int stolen_tasks);
};

} }


