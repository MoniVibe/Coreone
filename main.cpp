#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "engine/sim/sim_loop.hpp"

static bool parse_flag(const char* arg, const char* name, const char** value_out){
  const size_t name_len = std::strlen(name);
  if(std::strncmp(arg, name, name_len) != 0) return false;
  if(arg[name_len] == '='){ *value_out = arg + name_len + 1; return true; }
  return false;
}

int main(int argc, char** argv){
  eng::sim::Config cfg{};
  for(int i=1;i<argc;++i){
    const char* val=nullptr;
    if(parse_flag(argv[i], "--frames", &val) && val){ cfg.frames = std::atoi(val); continue; }
    if(parse_flag(argv[i], "--fixed-dt", &val) && val){ cfg.fixedDt = static_cast<float>(std::atof(val)); continue; }
    if(parse_flag(argv[i], "--seed", &val) && val){ cfg.seed = static_cast<unsigned>(std::strtoul(val, nullptr, 10)); continue; }
    if(parse_flag(argv[i], "--spawn", &val) && val){ cfg.spawn = std::atoi(val); continue; }
    if(parse_flag(argv[i], "--profile", &val)){
      cfg.profile = (val && (std::strcmp(val, "on") == 0 || std::strcmp(val, "1") == 0));
      continue;
    }
    if(std::strcmp(argv[i], "--scalar") == 0){ cfg.scalarOnly = true; continue; }
    if(std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0){
      std::puts("engine_app options:\n  --frames=N\n  --fixed-dt=F\n  --seed=S\n  --spawn=N\n  --profile=on|off\n  --scalar");
      return 0;
    }
  }
  eng::sim::run(cfg);
  return 0;
}


