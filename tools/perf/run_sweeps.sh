#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
APP="$ROOT/build/linux-clang/engine_app"
OUT="$ROOT/artifacts/perf/$(date +%Y%m%d)"
mkdir -p "$OUT"
for N in 1000 10000 50000 100000; do
  "$APP" --spawn $N --frames 200 --fixed-dt 0.016 --profile on > "$OUT/sweep_${N}.csv"
done


