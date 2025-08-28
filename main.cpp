#include <cstdio>
#include "engine/sim/sim_loop.hpp"

int main(int argc, char** argv){
  // TODO(cursor): parse --frames, --fixed-dt, --seed, --profile, --spawn
  eng::sim::Config cfg{};
  std::puts("engine_app stub");
  eng::sim::run(cfg);
  return 0;
}


