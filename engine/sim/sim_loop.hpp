#pragma once
namespace eng { namespace sim { struct Config { int frames{0}; float fixedDt{0.016f}; int spawn{0}; unsigned seed{0}; }; void run(const Config& cfg); } }


